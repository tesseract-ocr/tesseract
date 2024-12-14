import './node-environment';
import './require-hook';
import './node-polyfill-crypto';
import { DecodeError, PageNotFoundError, MiddlewareNotFoundError } from '../shared/lib/utils';
import fs from 'fs';
import { join, resolve } from 'path';
import { getRouteMatcher } from '../shared/lib/router/utils/route-matcher';
import { addRequestMeta, getRequestMeta } from './request-meta';
import { PAGES_MANIFEST, BUILD_ID_FILE, MIDDLEWARE_MANIFEST, PRERENDER_MANIFEST, ROUTES_MANIFEST, CLIENT_PUBLIC_FILES_PATH, APP_PATHS_MANIFEST, SERVER_DIRECTORY, NEXT_FONT_MANIFEST, PHASE_PRODUCTION_BUILD, UNDERSCORE_NOT_FOUND_ROUTE_ENTRY } from '../shared/lib/constants';
import { findDir } from '../lib/find-pages-dir';
import { NodeNextRequest, NodeNextResponse } from './base-http/node';
import { sendRenderResult } from './send-payload';
import { parseUrl } from '../shared/lib/router/utils/parse-url';
import * as Log from '../build/output/log';
import BaseServer, { NoFallbackError } from './base-server';
import { getMaybePagePath, getPagePath } from './require';
import { denormalizePagePath } from '../shared/lib/page-path/denormalize-page-path';
import { normalizePagePath } from '../shared/lib/page-path/normalize-page-path';
import { loadComponents } from './load-components';
import isError, { getProperError } from '../lib/is-error';
import { splitCookiesString, toNodeOutgoingHttpHeaders } from './web/utils';
import { getMiddlewareRouteMatcher } from '../shared/lib/router/utils/middleware-route-matcher';
import { loadEnvConfig } from '@next/env';
import { urlQueryToSearchParams } from '../shared/lib/router/utils/querystring';
import { removeTrailingSlash } from '../shared/lib/router/utils/remove-trailing-slash';
import { getNextPathnameInfo } from '../shared/lib/router/utils/get-next-pathname-info';
import { getCloneableBody } from './body-streams';
import { checkIsOnDemandRevalidate } from './api-utils';
import ResponseCache, { CachedRouteKind } from './response-cache';
import { IncrementalCache } from './lib/incremental-cache';
import { normalizeAppPath } from '../shared/lib/router/utils/app-paths';
import { setHttpClientAndAgentOptions } from './setup-http-agent-env';
import { isPagesAPIRouteMatch } from './route-matches/pages-api-route-match';
import { INSTRUMENTATION_HOOK_FILENAME } from '../lib/constants';
import { BubbledError, getTracer } from './lib/trace/tracer';
import { NextNodeServerSpan } from './lib/trace/constants';
import { nodeFs } from './lib/node-fs-methods';
import { getRouteRegex } from '../shared/lib/router/utils/route-regex';
import { pipeToNodeResponse } from './pipe-readable';
import { createRequestResponseMocks } from './lib/mock-request';
import { NEXT_RSC_UNION_QUERY } from '../client/components/app-router-headers';
import { signalFromNodeResponse } from './web/spec-extension/adapters/next-request';
import { RouteModuleLoader } from './lib/module-loader/route-module-loader';
import { loadManifest } from './load-manifest';
import { lazyRenderAppPage } from './route-modules/app-page/module.render';
import { lazyRenderPagesPage } from './route-modules/pages/module.render';
import { interopDefault } from '../lib/interop-default';
import { formatDynamicImportPath } from '../lib/format-dynamic-import-path';
import { isInterceptionRouteRewrite } from '../lib/generate-interception-routes-rewrites';
import { RouteKind } from './route-kind';
import { InvariantError } from '../shared/lib/invariant-error';
import { AwaiterOnce } from './after/awaiter';
import { AsyncCallbackSet } from './lib/async-callback-set';
export * from './base-server';
// For module that can be both CJS or ESM
const dynamicImportEsmDefault = process.env.NEXT_MINIMAL ? (id)=>import(/* webpackIgnore: true */ id).then((mod)=>mod.default || mod) : (id)=>import(id).then((mod)=>mod.default || mod);
// For module that will be compiled to CJS, e.g. instrument
const dynamicRequire = process.env.NEXT_MINIMAL ? __non_webpack_require__ : require;
const MiddlewareMatcherCache = new WeakMap();
function getMiddlewareMatcher(info) {
    const stored = MiddlewareMatcherCache.get(info);
    if (stored) {
        return stored;
    }
    if (!Array.isArray(info.matchers)) {
        throw new Error(`Invariant: invalid matchers for middleware ${JSON.stringify(info)}`);
    }
    const matcher = getMiddlewareRouteMatcher(info.matchers);
    MiddlewareMatcherCache.set(info, matcher);
    return matcher;
}
export default class NextNodeServer extends BaseServer {
    constructor(options){
        // Initialize super class
        super(options), this.registeredInstrumentation = false, this.cleanupListeners = new AsyncCallbackSet(), this.handleNextImageRequest = async (req, res, parsedUrl)=>{
            if (!parsedUrl.pathname || !parsedUrl.pathname.startsWith('/_next/image')) {
                return false;
            }
            if (this.minimalMode || this.nextConfig.output === 'export' || process.env.NEXT_MINIMAL) {
                res.statusCode = 400;
                res.body('Bad Request').send();
                return true;
            // the `else` branch is needed for tree-shaking
            } else {
                const { ImageOptimizerCache } = require('./image-optimizer');
                const imageOptimizerCache = new ImageOptimizerCache({
                    distDir: this.distDir,
                    nextConfig: this.nextConfig
                });
                const { sendResponse, ImageError } = require('./image-optimizer');
                if (!this.imageResponseCache) {
                    throw new Error('invariant image optimizer cache was not initialized');
                }
                const imagesConfig = this.nextConfig.images;
                if (imagesConfig.loader !== 'default' || imagesConfig.unoptimized) {
                    await this.render404(req, res);
                    return true;
                }
                const paramsResult = ImageOptimizerCache.validateParams(req.originalRequest, parsedUrl.query, this.nextConfig, !!this.renderOpts.dev);
                if ('errorMessage' in paramsResult) {
                    res.statusCode = 400;
                    res.body(paramsResult.errorMessage).send();
                    return true;
                }
                const cacheKey = ImageOptimizerCache.getCacheKey(paramsResult);
                try {
                    var _cacheEntry_value;
                    const { getExtension } = require('./serve-static');
                    const cacheEntry = await this.imageResponseCache.get(cacheKey, async ({ previousCacheEntry })=>{
                        const { buffer, contentType, maxAge, upstreamEtag, etag } = await this.imageOptimizer(req, res, paramsResult, previousCacheEntry);
                        return {
                            value: {
                                kind: CachedRouteKind.IMAGE,
                                buffer,
                                etag,
                                extension: getExtension(contentType),
                                upstreamEtag
                            },
                            isFallback: false,
                            revalidate: maxAge
                        };
                    }, {
                        routeKind: RouteKind.IMAGE,
                        incrementalCache: imageOptimizerCache,
                        isFallback: false
                    });
                    if ((cacheEntry == null ? void 0 : (_cacheEntry_value = cacheEntry.value) == null ? void 0 : _cacheEntry_value.kind) !== CachedRouteKind.IMAGE) {
                        throw new Error('invariant did not get entry from image response cache');
                    }
                    sendResponse(req.originalRequest, res.originalResponse, paramsResult.href, cacheEntry.value.extension, cacheEntry.value.buffer, cacheEntry.value.etag, paramsResult.isStatic, cacheEntry.isMiss ? 'MISS' : cacheEntry.isStale ? 'STALE' : 'HIT', imagesConfig, cacheEntry.revalidate || 0, Boolean(this.renderOpts.dev));
                    return true;
                } catch (err) {
                    if (err instanceof ImageError) {
                        res.statusCode = err.statusCode;
                        res.body(err.message).send();
                        return true;
                    }
                    throw err;
                }
            }
        }, this.handleCatchallRenderRequest = async (req, res, parsedUrl)=>{
            let { pathname, query } = parsedUrl;
            if (!pathname) {
                throw new Error('Invariant: pathname is undefined');
            }
            // This is a catch-all route, there should be no fallbacks so mark it as
            // such.
            query._nextBubbleNoFallback = '1';
            try {
                var _this_i18nProvider;
                // next.js core assumes page path without trailing slash
                pathname = removeTrailingSlash(pathname);
                const options = {
                    i18n: (_this_i18nProvider = this.i18nProvider) == null ? void 0 : _this_i18nProvider.fromQuery(pathname, query)
                };
                const match = await this.matchers.match(pathname, options);
                // If we don't have a match, try to render it anyways.
                if (!match) {
                    await this.render(req, res, pathname, query, parsedUrl, true);
                    return true;
                }
                // Add the match to the request so we don't have to re-run the matcher
                // for the same request.
                addRequestMeta(req, 'match', match);
                // TODO-APP: move this to a route handler
                const edgeFunctionsPages = this.getEdgeFunctionsPages();
                for (const edgeFunctionsPage of edgeFunctionsPages){
                    // If the page doesn't match the edge function page, skip it.
                    if (edgeFunctionsPage !== match.definition.page) continue;
                    if (this.nextConfig.output === 'export') {
                        await this.render404(req, res, parsedUrl);
                        return true;
                    }
                    delete query._nextBubbleNoFallback;
                    delete query[NEXT_RSC_UNION_QUERY];
                    // If we handled the request, we can return early.
                    // For api routes edge runtime
                    try {
                        const handled = await this.runEdgeFunction({
                            req,
                            res,
                            query,
                            params: match.params,
                            page: match.definition.page,
                            match,
                            appPaths: null
                        });
                        if (handled) return true;
                    } catch (apiError) {
                        await this.instrumentationOnRequestError(apiError, req, {
                            routePath: match.definition.page,
                            routerKind: 'Pages Router',
                            routeType: 'route',
                            // Edge runtime does not support ISR
                            revalidateReason: undefined
                        });
                        throw apiError;
                    }
                }
                // If the route was detected as being a Pages API route, then handle
                // it.
                // TODO: move this behavior into a route handler.
                if (isPagesAPIRouteMatch(match)) {
                    if (this.nextConfig.output === 'export') {
                        await this.render404(req, res, parsedUrl);
                        return true;
                    }
                    delete query._nextBubbleNoFallback;
                    const handled = await this.handleApiRequest(req, res, query, match);
                    if (handled) return true;
                }
                await this.render(req, res, pathname, query, parsedUrl, true);
                return true;
            } catch (err) {
                if (err instanceof NoFallbackError) {
                    throw err;
                }
                try {
                    if (this.renderOpts.dev) {
                        const { formatServerError } = require('../lib/format-server-error');
                        formatServerError(err);
                        this.logErrorWithOriginalStack(err);
                    } else {
                        this.logError(err);
                    }
                    res.statusCode = 500;
                    await this.renderError(err, req, res, pathname, query);
                    return true;
                } catch  {}
                throw err;
            }
        }, this.handleCatchallMiddlewareRequest = async (req, res, parsed)=>{
            const isMiddlewareInvoke = getRequestMeta(req, 'middlewareInvoke');
            if (!isMiddlewareInvoke) {
                return false;
            }
            const handleFinished = ()=>{
                addRequestMeta(req, 'middlewareInvoke', true);
                res.body('').send();
                return true;
            };
            const middleware = this.getMiddleware();
            if (!middleware) {
                return handleFinished();
            }
            const initUrl = getRequestMeta(req, 'initURL');
            const parsedUrl = parseUrl(initUrl);
            const pathnameInfo = getNextPathnameInfo(parsedUrl.pathname, {
                nextConfig: this.nextConfig,
                i18nProvider: this.i18nProvider
            });
            parsedUrl.pathname = pathnameInfo.pathname;
            const normalizedPathname = removeTrailingSlash(parsed.pathname || '');
            if (!middleware.match(normalizedPathname, req, parsedUrl.query)) {
                return handleFinished();
            }
            let result;
            let bubblingResult = false;
            try {
                await this.ensureMiddleware(req.url);
                result = await this.runMiddleware({
                    request: req,
                    response: res,
                    parsedUrl: parsedUrl,
                    parsed: parsed
                });
                if ('response' in result) {
                    if (isMiddlewareInvoke) {
                        bubblingResult = true;
                        throw new BubbledError(true, result);
                    }
                    for (const [key, value] of Object.entries(toNodeOutgoingHttpHeaders(result.response.headers))){
                        if (key !== 'content-encoding' && value !== undefined) {
                            res.setHeader(key, value);
                        }
                    }
                    res.statusCode = result.response.status;
                    const { originalResponse } = res;
                    if (result.response.body) {
                        await pipeToNodeResponse(result.response.body, originalResponse);
                    } else {
                        originalResponse.end();
                    }
                    return true;
                }
            } catch (err) {
                if (bubblingResult) {
                    throw err;
                }
                if (isError(err) && err.code === 'ENOENT') {
                    await this.render404(req, res, parsed);
                    return true;
                }
                if (err instanceof DecodeError) {
                    res.statusCode = 400;
                    await this.renderError(err, req, res, parsed.pathname || '');
                    return true;
                }
                const error = getProperError(err);
                console.error(error);
                res.statusCode = 500;
                await this.renderError(error, req, res, parsed.pathname || '');
                return true;
            }
            return result.finished;
        };
        /**
     * This sets environment variable to be used at the time of SSR by head.tsx.
     * Using this from process.env allows targeting SSR by calling
     * `process.env.__NEXT_OPTIMIZE_CSS`.
     */ if (this.renderOpts.optimizeCss) {
            process.env.__NEXT_OPTIMIZE_CSS = JSON.stringify(true);
        }
        if (this.renderOpts.nextScriptWorkers) {
            process.env.__NEXT_SCRIPT_WORKERS = JSON.stringify(true);
        }
        process.env.NEXT_DEPLOYMENT_ID = this.nextConfig.deploymentId || '';
        if (!this.minimalMode) {
            this.imageResponseCache = new ResponseCache(this.minimalMode);
        }
        const { appDocumentPreloading } = this.nextConfig.experimental;
        const isDefaultEnabled = typeof appDocumentPreloading === 'undefined';
        if (!options.dev && (appDocumentPreloading === true || !(this.minimalMode && isDefaultEnabled))) {
            // pre-warm _document and _app as these will be
            // needed for most requests
            loadComponents({
                distDir: this.distDir,
                page: '/_document',
                isAppPath: false
            }).catch(()=>{});
            loadComponents({
                distDir: this.distDir,
                page: '/_app',
                isAppPath: false
            }).catch(()=>{});
        }
        if (!options.dev && !this.minimalMode && this.nextConfig.experimental.preloadEntriesOnStart) {
            this.unstable_preloadEntries();
        }
        if (!options.dev) {
            const { dynamicRoutes = [] } = this.getRoutesManifest() ?? {};
            this.dynamicRoutes = dynamicRoutes.map((r)=>{
                // TODO: can we just re-use the regex from the manifest?
                const regex = getRouteRegex(r.page);
                const match = getRouteMatcher(regex);
                return {
                    match,
                    page: r.page,
                    re: regex.re
                };
            });
        }
        // ensure options are set when loadConfig isn't called
        setHttpClientAndAgentOptions(this.nextConfig);
        // Intercept fetch and other testmode apis.
        if (this.serverOptions.experimentalTestProxy) {
            process.env.NEXT_PRIVATE_TEST_PROXY = 'true';
            const { interceptTestApis } = require('next/dist/experimental/testmode/server');
            interceptTestApis();
        }
        this.middlewareManifestPath = join(this.serverDistDir, MIDDLEWARE_MANIFEST);
        // This is just optimization to fire prepare as soon as possible. It will be
        // properly awaited later. We add the catch here to ensure that it does not
        // cause a unhandled promise rejection. The promise rejection will be
        // handled later on via the `await` when the request handler is called.
        if (!options.dev) {
            this.prepare().catch((err)=>{
                console.error('Failed to prepare server', err);
            });
        }
    }
    async unstable_preloadEntries() {
        const appPathsManifest = this.getAppPathsManifest();
        const pagesManifest = this.getPagesManifest();
        for (const page of Object.keys(pagesManifest || {})){
            await loadComponents({
                distDir: this.distDir,
                page,
                isAppPath: false
            }).catch(()=>{});
        }
        for (const page of Object.keys(appPathsManifest || {})){
            await loadComponents({
                distDir: this.distDir,
                page,
                isAppPath: true
            }).then(async ({ ComponentMod })=>{
                // we need to ensure fetch is patched before we require the page,
                // otherwise if the fetch is patched by user code, we will be patching it
                // too late and there won't be any caching behaviors
                ComponentMod.patchFetch();
                const webpackRequire = ComponentMod.__next_app__.require;
                if (webpackRequire == null ? void 0 : webpackRequire.m) {
                    for (const id of Object.keys(webpackRequire.m)){
                        await webpackRequire(id);
                    }
                }
            }).catch(()=>{});
        }
    }
    async handleUpgrade() {
    // The web server does not support web sockets, it's only used for HMR in
    // development.
    }
    async loadInstrumentationModule() {
        if (!this.serverOptions.dev) {
            try {
                this.instrumentation = await dynamicRequire(resolve(this.serverOptions.dir || '.', this.serverOptions.conf.distDir, 'server', INSTRUMENTATION_HOOK_FILENAME));
            } catch (err) {
                if (err.code !== 'MODULE_NOT_FOUND') {
                    throw new Error('An error occurred while loading the instrumentation hook', {
                        cause: err
                    });
                }
            }
        }
        return this.instrumentation;
    }
    async prepareImpl() {
        await super.prepareImpl();
        await this.runInstrumentationHookIfAvailable();
    }
    async runInstrumentationHookIfAvailable() {
        var _this_instrumentation_register, _this_instrumentation;
        if (this.registeredInstrumentation) return;
        this.registeredInstrumentation = true;
        await ((_this_instrumentation = this.instrumentation) == null ? void 0 : (_this_instrumentation_register = _this_instrumentation.register) == null ? void 0 : _this_instrumentation_register.call(_this_instrumentation));
    }
    loadEnvConfig({ dev, forceReload, silent }) {
        loadEnvConfig(this.dir, dev, silent ? {
            info: ()=>{},
            error: ()=>{}
        } : Log, forceReload);
    }
    async getIncrementalCache({ requestHeaders, requestProtocol }) {
        const dev = !!this.renderOpts.dev;
        let CacheHandler;
        const { cacheHandler } = this.nextConfig;
        if (cacheHandler) {
            CacheHandler = interopDefault(await dynamicImportEsmDefault(formatDynamicImportPath(this.distDir, cacheHandler)));
        }
        const { cacheHandlers } = this.nextConfig.experimental;
        if (!globalThis.__nextCacheHandlers && cacheHandlers) {
            ;
            globalThis.__nextCacheHandlers = {};
            for (const key of Object.keys(cacheHandlers)){
                if (cacheHandlers[key]) {
                    ;
                    globalThis.__nextCacheHandlers[key] = interopDefault(await dynamicImportEsmDefault(formatDynamicImportPath(this.distDir, cacheHandlers[key])));
                }
            }
        }
        // incremental-cache is request specific
        // although can have shared caches in module scope
        // per-cache handler
        return new IncrementalCache({
            fs: this.getCacheFilesystem(),
            dev,
            requestHeaders,
            requestProtocol,
            dynamicIO: Boolean(this.nextConfig.experimental.dynamicIO),
            allowedRevalidateHeaderKeys: this.nextConfig.experimental.allowedRevalidateHeaderKeys,
            minimalMode: this.minimalMode,
            serverDistDir: this.serverDistDir,
            fetchCache: true,
            fetchCacheKeyPrefix: this.nextConfig.experimental.fetchCacheKeyPrefix,
            maxMemoryCacheSize: this.nextConfig.cacheMaxMemorySize,
            flushToDisk: !this.minimalMode && this.nextConfig.experimental.isrFlushToDisk,
            getPrerenderManifest: ()=>this.getPrerenderManifest(),
            CurCacheHandler: CacheHandler
        });
    }
    getResponseCache() {
        return new ResponseCache(this.minimalMode);
    }
    getPublicDir() {
        return join(this.dir, CLIENT_PUBLIC_FILES_PATH);
    }
    getHasStaticDir() {
        return fs.existsSync(join(this.dir, 'static'));
    }
    getPagesManifest() {
        return loadManifest(join(this.serverDistDir, PAGES_MANIFEST));
    }
    getAppPathsManifest() {
        if (!this.enabledDirectories.app) return undefined;
        return loadManifest(join(this.serverDistDir, APP_PATHS_MANIFEST));
    }
    getinterceptionRoutePatterns() {
        if (!this.enabledDirectories.app) return [];
        const routesManifest = this.getRoutesManifest();
        return (routesManifest == null ? void 0 : routesManifest.rewrites.beforeFiles.filter(isInterceptionRouteRewrite).map((rewrite)=>new RegExp(rewrite.regex))) ?? [];
    }
    async hasPage(pathname) {
        var _this_nextConfig_i18n;
        return !!getMaybePagePath(pathname, this.distDir, (_this_nextConfig_i18n = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n.locales, this.enabledDirectories.app);
    }
    getBuildId() {
        const buildIdFile = join(this.distDir, BUILD_ID_FILE);
        try {
            return fs.readFileSync(buildIdFile, 'utf8').trim();
        } catch (err) {
            if (err.code === 'ENOENT') {
                throw new Error(`Could not find a production build in the '${this.distDir}' directory. Try building your app with 'next build' before starting the production server. https://nextjs.org/docs/messages/production-start-no-build-id`);
            }
            throw err;
        }
    }
    getEnabledDirectories(dev) {
        const dir = dev ? this.dir : this.serverDistDir;
        return {
            app: findDir(dir, 'app') ? true : false,
            pages: findDir(dir, 'pages') ? true : false
        };
    }
    sendRenderResult(req, res, options) {
        return sendRenderResult({
            req: req.originalRequest,
            res: res.originalResponse,
            result: options.result,
            type: options.type,
            generateEtags: options.generateEtags,
            poweredByHeader: options.poweredByHeader,
            revalidate: options.revalidate,
            expireTime: options.expireTime
        });
    }
    async runApi(req, res, query, match) {
        const edgeFunctionsPages = this.getEdgeFunctionsPages();
        for (const edgeFunctionsPage of edgeFunctionsPages){
            if (edgeFunctionsPage === match.definition.pathname) {
                const handledAsEdgeFunction = await this.runEdgeFunction({
                    req,
                    res,
                    query,
                    params: match.params,
                    page: match.definition.pathname,
                    appPaths: null
                });
                if (handledAsEdgeFunction) {
                    return true;
                }
            }
        }
        // The module supports minimal mode, load the minimal module.
        const module = await RouteModuleLoader.load(match.definition.filename);
        query = {
            ...query,
            ...match.params
        };
        delete query.__nextLocale;
        delete query.__nextDefaultLocale;
        delete query.__nextInferredLocaleFromDefault;
        await module.render(req.originalRequest, res.originalResponse, {
            previewProps: this.renderOpts.previewProps,
            revalidate: this.revalidate.bind(this),
            trustHostHeader: this.nextConfig.experimental.trustHostHeader,
            allowedRevalidateHeaderKeys: this.nextConfig.experimental.allowedRevalidateHeaderKeys,
            hostname: this.fetchHostname,
            minimalMode: this.minimalMode,
            dev: this.renderOpts.dev === true,
            query,
            params: match.params,
            page: match.definition.pathname,
            onError: this.instrumentationOnRequestError.bind(this),
            multiZoneDraftMode: this.nextConfig.experimental.multiZoneDraftMode
        });
        return true;
    }
    async renderHTML(req, res, pathname, query, renderOpts) {
        return getTracer().trace(NextNodeServerSpan.renderHTML, async ()=>this.renderHTMLImpl(req, res, pathname, query, renderOpts));
    }
    async renderHTMLImpl(req, res, pathname, query, renderOpts) {
        if (process.env.NEXT_MINIMAL) {
            throw new Error('Invariant: renderHTML should not be called in minimal mode');
        // the `else` branch is needed for tree-shaking
        } else {
            // Due to the way we pass data by mutating `renderOpts`, we can't extend the
            // object here but only updating its `nextFontManifest` field.
            // https://github.com/vercel/next.js/blob/df7cbd904c3bd85f399d1ce90680c0ecf92d2752/packages/next/server/render.tsx#L947-L952
            renderOpts.nextFontManifest = this.nextFontManifest;
            if (this.enabledDirectories.app && renderOpts.isAppPath) {
                return lazyRenderAppPage(req, res, pathname, query, // This code path does not service revalidations for unknown param
                // shells. As a result, we don't need to pass in the unknown params.
                null, renderOpts, this.getServerComponentsHmrCache(), false);
            }
            // TODO: re-enable this once we've refactored to use implicit matches
            // throw new Error('Invariant: render should have used routeModule')
            return lazyRenderPagesPage(req.originalRequest, res.originalResponse, pathname, query, renderOpts);
        }
    }
    async imageOptimizer(req, res, paramsResult, previousCacheEntry) {
        if (process.env.NEXT_MINIMAL) {
            throw new Error('invariant: imageOptimizer should not be called in minimal mode');
        } else {
            const { imageOptimizer, fetchExternalImage, fetchInternalImage } = require('./image-optimizer');
            const handleInternalReq = async (newReq, newRes)=>{
                if (newReq.url === req.url) {
                    throw new Error(`Invariant attempted to optimize _next/image itself`);
                }
                if (!this.routerServerHandler) {
                    throw new Error(`Invariant missing routerServerHandler`);
                }
                await this.routerServerHandler(newReq, newRes);
                return;
            };
            const { isAbsolute, href } = paramsResult;
            const imageUpstream = isAbsolute ? await fetchExternalImage(href) : await fetchInternalImage(href, req.originalRequest, res.originalResponse, handleInternalReq);
            return imageOptimizer(imageUpstream, paramsResult, this.nextConfig, {
                isDev: this.renderOpts.dev,
                previousCacheEntry
            });
        }
    }
    getPagePath(pathname, locales) {
        return getPagePath(pathname, this.distDir, locales, this.enabledDirectories.app);
    }
    async renderPageComponent(ctx, bubbleNoFallback) {
        const edgeFunctionsPages = this.getEdgeFunctionsPages() || [];
        if (edgeFunctionsPages.length) {
            const appPaths = this.getOriginalAppPaths(ctx.pathname);
            const isAppPath = Array.isArray(appPaths);
            let page = ctx.pathname;
            if (isAppPath) {
                // When it's an array, we need to pass all parallel routes to the loader.
                page = appPaths[0];
            }
            for (const edgeFunctionsPage of edgeFunctionsPages){
                if (edgeFunctionsPage === page) {
                    await this.runEdgeFunction({
                        req: ctx.req,
                        res: ctx.res,
                        query: ctx.query,
                        params: ctx.renderOpts.params,
                        page,
                        appPaths
                    });
                    return null;
                }
            }
        }
        return super.renderPageComponent(ctx, bubbleNoFallback);
    }
    async findPageComponents({ page, query, params, isAppPath, url }) {
        return getTracer().trace(NextNodeServerSpan.findPageComponents, {
            spanName: 'resolve page components',
            attributes: {
                'next.route': isAppPath ? normalizeAppPath(page) : page
            }
        }, ()=>this.findPageComponentsImpl({
                page,
                query,
                params,
                isAppPath,
                url
            }));
    }
    async findPageComponentsImpl({ page, query, params, isAppPath, url: _url }) {
        const pagePaths = [
            page
        ];
        if (query.amp) {
            // try serving a static AMP version first
            pagePaths.unshift((isAppPath ? normalizeAppPath(page) : normalizePagePath(page)) + '.amp');
        }
        if (query.__nextLocale) {
            pagePaths.unshift(...pagePaths.map((path)=>`/${query.__nextLocale}${path === '/' ? '' : path}`));
        }
        for (const pagePath of pagePaths){
            try {
                const components = await loadComponents({
                    distDir: this.distDir,
                    page: pagePath,
                    isAppPath
                });
                if (query.__nextLocale && typeof components.Component === 'string' && !pagePath.startsWith(`/${query.__nextLocale}`)) {
                    continue;
                }
                return {
                    components,
                    query: {
                        ...!this.renderOpts.isExperimentalCompile && components.getStaticProps ? {
                            amp: query.amp,
                            __nextDataReq: query.__nextDataReq,
                            __nextLocale: query.__nextLocale,
                            __nextDefaultLocale: query.__nextDefaultLocale
                        } : query,
                        // For appDir params is excluded.
                        ...(isAppPath ? {} : params) || {}
                    }
                };
            } catch (err) {
                // we should only not throw if we failed to find the page
                // in the pages-manifest
                if (!(err instanceof PageNotFoundError)) {
                    throw err;
                }
            }
        }
        return null;
    }
    getNextFontManifest() {
        return loadManifest(join(this.distDir, 'server', NEXT_FONT_MANIFEST + '.json'));
    }
    // Used in development only, overloaded in next-dev-server
    logErrorWithOriginalStack(_err, _type) {
        throw new Error('Invariant: logErrorWithOriginalStack can only be called on the development server');
    }
    // Used in development only, overloaded in next-dev-server
    async ensurePage(_opts) {
        throw new Error('Invariant: ensurePage can only be called on the development server');
    }
    /**
   * Resolves `API` request, in development builds on demand
   * @param req http request
   * @param res http response
   * @param pathname path of request
   */ async handleApiRequest(req, res, query, match) {
        return this.runApi(req, res, query, match);
    }
    getCacheFilesystem() {
        return nodeFs;
    }
    normalizeReq(req) {
        return !(req instanceof NodeNextRequest) ? new NodeNextRequest(req) : req;
    }
    normalizeRes(res) {
        return !(res instanceof NodeNextResponse) ? new NodeNextResponse(res) : res;
    }
    getRequestHandler() {
        const handler = this.makeRequestHandler();
        if (this.serverOptions.experimentalTestProxy) {
            const { wrapRequestHandlerNode } = require('next/dist/experimental/testmode/server');
            return wrapRequestHandlerNode(handler);
        }
        return handler;
    }
    makeRequestHandler() {
        // This is just optimization to fire prepare as soon as possible. It will be
        // properly awaited later. We add the catch here to ensure that it does not
        // cause an unhandled promise rejection. The promise rejection will be
        // handled later on via the `await` when the request handler is called.
        this.prepare().catch((err)=>{
            console.error('Failed to prepare server', err);
        });
        const handler = super.getRequestHandler();
        return (req, res, parsedUrl)=>handler(this.normalizeReq(req), this.normalizeRes(res), parsedUrl);
    }
    async revalidate({ urlPath, revalidateHeaders, opts }) {
        const mocked = createRequestResponseMocks({
            url: urlPath,
            headers: revalidateHeaders
        });
        const handler = this.getRequestHandler();
        await handler(new NodeNextRequest(mocked.req), new NodeNextResponse(mocked.res));
        await mocked.res.hasStreamed;
        if (mocked.res.getHeader('x-nextjs-cache') !== 'REVALIDATED' && !(mocked.res.statusCode === 404 && opts.unstable_onlyGenerated)) {
            throw new Error(`Invalid response ${mocked.res.statusCode}`);
        }
    }
    async render(req, res, pathname, query, parsedUrl, internal = false) {
        return super.render(this.normalizeReq(req), this.normalizeRes(res), pathname, query, parsedUrl, internal);
    }
    async renderToHTML(req, res, pathname, query) {
        return super.renderToHTML(this.normalizeReq(req), this.normalizeRes(res), pathname, query);
    }
    async renderErrorToResponseImpl(ctx, err) {
        const { req, res, query } = ctx;
        const is404 = res.statusCode === 404;
        if (is404 && this.enabledDirectories.app) {
            if (this.renderOpts.dev) {
                await this.ensurePage({
                    page: UNDERSCORE_NOT_FOUND_ROUTE_ENTRY,
                    clientOnly: false,
                    url: req.url
                }).catch(()=>{});
            }
            if (this.getEdgeFunctionsPages().includes(UNDERSCORE_NOT_FOUND_ROUTE_ENTRY)) {
                await this.runEdgeFunction({
                    req,
                    res,
                    query: query || {},
                    params: {},
                    page: UNDERSCORE_NOT_FOUND_ROUTE_ENTRY,
                    appPaths: null
                });
                return null;
            }
        }
        return super.renderErrorToResponseImpl(ctx, err);
    }
    async renderError(err, req, res, pathname, query, setHeaders) {
        return super.renderError(err, this.normalizeReq(req), this.normalizeRes(res), pathname, query, setHeaders);
    }
    async renderErrorToHTML(err, req, res, pathname, query) {
        return super.renderErrorToHTML(err, this.normalizeReq(req), this.normalizeRes(res), pathname, query);
    }
    async render404(req, res, parsedUrl, setHeaders) {
        return super.render404(this.normalizeReq(req), this.normalizeRes(res), parsedUrl, setHeaders);
    }
    getMiddlewareManifest() {
        if (this.minimalMode) return null;
        const manifest = require(this.middlewareManifestPath);
        return manifest;
    }
    /** Returns the middleware routing item if there is one. */ getMiddleware() {
        var _manifest_middleware;
        const manifest = this.getMiddlewareManifest();
        const middleware = manifest == null ? void 0 : (_manifest_middleware = manifest.middleware) == null ? void 0 : _manifest_middleware['/'];
        if (!middleware) {
            return;
        }
        return {
            match: getMiddlewareMatcher(middleware),
            page: '/'
        };
    }
    getEdgeFunctionsPages() {
        const manifest = this.getMiddlewareManifest();
        if (!manifest) {
            return [];
        }
        return Object.keys(manifest.functions);
    }
    /**
   * Get information for the edge function located in the provided page
   * folder. If the edge function info can't be found it will throw
   * an error.
   */ getEdgeFunctionInfo(params) {
        const manifest = this.getMiddlewareManifest();
        if (!manifest) {
            return null;
        }
        let foundPage;
        try {
            foundPage = denormalizePagePath(normalizePagePath(params.page));
        } catch (err) {
            return null;
        }
        let pageInfo = params.middleware ? manifest.middleware[foundPage] : manifest.functions[foundPage];
        if (!pageInfo) {
            if (!params.middleware) {
                throw new PageNotFoundError(foundPage);
            }
            return null;
        }
        return {
            name: pageInfo.name,
            paths: pageInfo.files.map((file)=>join(this.distDir, file)),
            wasm: (pageInfo.wasm ?? []).map((binding)=>({
                    ...binding,
                    filePath: join(this.distDir, binding.filePath)
                })),
            assets: pageInfo.assets && pageInfo.assets.map((binding)=>{
                return {
                    ...binding,
                    filePath: join(this.distDir, binding.filePath)
                };
            }),
            env: pageInfo.env
        };
    }
    /**
   * Checks if a middleware exists. This method is useful for the development
   * server where we need to check the filesystem. Here we just check the
   * middleware manifest.
   */ async hasMiddleware(pathname) {
        const info = this.getEdgeFunctionInfo({
            page: pathname,
            middleware: true
        });
        return Boolean(info && info.paths.length > 0);
    }
    /**
   * A placeholder for a function to be defined in the development server.
   * It will make sure that the root middleware or an edge function has been compiled
   * so that we can run it.
   */ async ensureMiddleware(_url) {}
    async ensureEdgeFunction(_params) {}
    /**
   * This method gets all middleware matchers and execute them when the request
   * matches. It will make sure that each middleware exists and is compiled and
   * ready to be invoked. The development server will decorate it to add warns
   * and errors with rich traces.
   */ async runMiddleware(params) {
        if (process.env.NEXT_MINIMAL) {
            throw new Error('invariant: runMiddleware should not be called in minimal mode');
        }
        // Middleware is skipped for on-demand revalidate requests
        if (checkIsOnDemandRevalidate(params.request, this.renderOpts.previewProps).isOnDemandRevalidate) {
            return {
                response: new Response(null, {
                    headers: {
                        'x-middleware-next': '1'
                    }
                })
            };
        }
        let url;
        if (this.nextConfig.skipMiddlewareUrlNormalize) {
            url = getRequestMeta(params.request, 'initURL');
        } else {
            // For middleware to "fetch" we must always provide an absolute URL
            const query = urlQueryToSearchParams(params.parsed.query).toString();
            const locale = params.parsed.query.__nextLocale;
            url = `${getRequestMeta(params.request, 'initProtocol')}://${this.fetchHostname || 'localhost'}:${this.port}${locale ? `/${locale}` : ''}${params.parsed.pathname}${query ? `?${query}` : ''}`;
        }
        if (!url.startsWith('http')) {
            throw new Error('To use middleware you must provide a `hostname` and `port` to the Next.js Server');
        }
        const page = {};
        const middleware = this.getMiddleware();
        if (!middleware) {
            return {
                finished: false
            };
        }
        if (!await this.hasMiddleware(middleware.page)) {
            return {
                finished: false
            };
        }
        await this.ensureMiddleware(params.request.url);
        const middlewareInfo = this.getEdgeFunctionInfo({
            page: middleware.page,
            middleware: true
        });
        if (!middlewareInfo) {
            throw new MiddlewareNotFoundError();
        }
        const method = (params.request.method || 'GET').toUpperCase();
        const { run } = require('./web/sandbox');
        const result = await run({
            distDir: this.distDir,
            name: middlewareInfo.name,
            paths: middlewareInfo.paths,
            edgeFunctionEntry: middlewareInfo,
            request: {
                headers: params.request.headers,
                method,
                nextConfig: {
                    basePath: this.nextConfig.basePath,
                    i18n: this.nextConfig.i18n,
                    trailingSlash: this.nextConfig.trailingSlash,
                    experimental: this.nextConfig.experimental
                },
                url: url,
                page,
                body: getRequestMeta(params.request, 'clonableBody'),
                signal: signalFromNodeResponse(params.response.originalResponse),
                waitUntil: this.getWaitUntil()
            },
            useCache: true,
            onWarning: params.onWarning
        });
        if (!this.renderOpts.dev) {
            result.waitUntil.catch((error)=>{
                console.error(`Uncaught: middleware waitUntil errored`, error);
            });
        }
        if (!result) {
            this.render404(params.request, params.response, params.parsed);
            return {
                finished: true
            };
        }
        // Split compound (comma-separated) set-cookie headers
        if (result.response.headers.has('set-cookie')) {
            const cookies = result.response.headers.getSetCookie().flatMap((maybeCompoundCookie)=>splitCookiesString(maybeCompoundCookie));
            // Clear existing header(s)
            result.response.headers.delete('set-cookie');
            // Append each cookie individually.
            for (const cookie of cookies){
                result.response.headers.append('set-cookie', cookie);
            }
            // Add cookies to request meta.
            addRequestMeta(params.request, 'middlewareCookie', cookies);
        }
        return result;
    }
    getPrerenderManifest() {
        var _this_renderOpts, _this_serverOptions;
        if (this._cachedPreviewManifest) {
            return this._cachedPreviewManifest;
        }
        if (((_this_renderOpts = this.renderOpts) == null ? void 0 : _this_renderOpts.dev) || ((_this_serverOptions = this.serverOptions) == null ? void 0 : _this_serverOptions.dev) || process.env.NODE_ENV === 'development' || process.env.NEXT_PHASE === PHASE_PRODUCTION_BUILD) {
            this._cachedPreviewManifest = {
                version: 4,
                routes: {},
                dynamicRoutes: {},
                notFoundRoutes: [],
                preview: {
                    previewModeId: require('crypto').randomBytes(16).toString('hex'),
                    previewModeSigningKey: require('crypto').randomBytes(32).toString('hex'),
                    previewModeEncryptionKey: require('crypto').randomBytes(32).toString('hex')
                }
            };
            return this._cachedPreviewManifest;
        }
        this._cachedPreviewManifest = loadManifest(join(this.distDir, PRERENDER_MANIFEST));
        return this._cachedPreviewManifest;
    }
    getRoutesManifest() {
        return getTracer().trace(NextNodeServerSpan.getRoutesManifest, ()=>{
            const manifest = loadManifest(join(this.distDir, ROUTES_MANIFEST));
            let rewrites = manifest.rewrites ?? {
                beforeFiles: [],
                afterFiles: [],
                fallback: []
            };
            if (Array.isArray(rewrites)) {
                rewrites = {
                    beforeFiles: [],
                    afterFiles: rewrites,
                    fallback: []
                };
            }
            return {
                ...manifest,
                rewrites
            };
        });
    }
    attachRequestMeta(req, parsedUrl, isUpgradeReq) {
        var _req_headers_xforwardedproto;
        // Injected in base-server.ts
        const protocol = ((_req_headers_xforwardedproto = req.headers['x-forwarded-proto']) == null ? void 0 : _req_headers_xforwardedproto.includes('https')) ? 'https' : 'http';
        // When there are hostname and port we build an absolute URL
        const initUrl = this.fetchHostname && this.port ? `${protocol}://${this.fetchHostname}:${this.port}${req.url}` : this.nextConfig.experimental.trustHostHeader ? `https://${req.headers.host || 'localhost'}${req.url}` : req.url;
        addRequestMeta(req, 'initURL', initUrl);
        addRequestMeta(req, 'initQuery', {
            ...parsedUrl.query
        });
        addRequestMeta(req, 'initProtocol', protocol);
        if (!isUpgradeReq) {
            addRequestMeta(req, 'clonableBody', getCloneableBody(req.originalRequest));
        }
    }
    async runEdgeFunction(params) {
        if (process.env.NEXT_MINIMAL) {
            throw new Error('Middleware is not supported in minimal mode. Please remove the `NEXT_MINIMAL` environment variable.');
        }
        let edgeInfo;
        const { query, page, match } = params;
        if (!match) await this.ensureEdgeFunction({
            page,
            appPaths: params.appPaths,
            url: params.req.url
        });
        edgeInfo = this.getEdgeFunctionInfo({
            page,
            middleware: false
        });
        if (!edgeInfo) {
            return null;
        }
        // For edge to "fetch" we must always provide an absolute URL
        const isNextDataRequest = !!query.__nextDataReq;
        const initialUrl = new URL(getRequestMeta(params.req, 'initURL') || '/', 'http://n');
        const queryString = urlQueryToSearchParams({
            ...Object.fromEntries(initialUrl.searchParams),
            ...query,
            ...params.params
        }).toString();
        if (isNextDataRequest) {
            params.req.headers['x-nextjs-data'] = '1';
        }
        initialUrl.search = queryString;
        const url = initialUrl.toString();
        if (!url.startsWith('http')) {
            throw new Error('To use middleware you must provide a `hostname` and `port` to the Next.js Server');
        }
        const { run } = require('./web/sandbox');
        const result = await run({
            distDir: this.distDir,
            name: edgeInfo.name,
            paths: edgeInfo.paths,
            edgeFunctionEntry: edgeInfo,
            request: {
                headers: params.req.headers,
                method: params.req.method,
                nextConfig: {
                    basePath: this.nextConfig.basePath,
                    i18n: this.nextConfig.i18n,
                    trailingSlash: this.nextConfig.trailingSlash
                },
                url,
                page: {
                    name: params.page,
                    ...params.params && {
                        params: params.params
                    }
                },
                body: getRequestMeta(params.req, 'clonableBody'),
                signal: signalFromNodeResponse(params.res.originalResponse),
                waitUntil: this.getWaitUntil()
            },
            useCache: true,
            onError: params.onError,
            onWarning: params.onWarning,
            incrementalCache: globalThis.__incrementalCache || getRequestMeta(params.req, 'incrementalCache'),
            serverComponentsHmrCache: getRequestMeta(params.req, 'serverComponentsHmrCache')
        });
        if (result.fetchMetrics) {
            params.req.fetchMetrics = result.fetchMetrics;
        }
        if (!params.res.statusCode || params.res.statusCode < 400) {
            params.res.statusCode = result.response.status;
            params.res.statusMessage = result.response.statusText;
        }
        // TODO: (wyattjoh) investigate improving this
        result.response.headers.forEach((value, key)=>{
            // The append handling is special cased for `set-cookie`.
            if (key.toLowerCase() === 'set-cookie') {
                // TODO: (wyattjoh) replace with native response iteration when we can upgrade undici
                for (const cookie of splitCookiesString(value)){
                    params.res.appendHeader(key, cookie);
                }
            } else {
                params.res.appendHeader(key, value);
            }
        });
        const { originalResponse } = params.res;
        if (result.response.body) {
            await pipeToNodeResponse(result.response.body, originalResponse);
        } else {
            originalResponse.end();
        }
        return result;
    }
    get serverDistDir() {
        if (this._serverDistDir) {
            return this._serverDistDir;
        }
        const serverDistDir = join(this.distDir, SERVER_DIRECTORY);
        this._serverDistDir = serverDistDir;
        return serverDistDir;
    }
    async getFallbackErrorComponents(_url) {
        // Not implemented for production use cases, this is implemented on the
        // development server.
        return null;
    }
    async instrumentationOnRequestError(...args) {
        await super.instrumentationOnRequestError(...args);
        // For Node.js runtime production logs, in dev it will be overridden by next-dev-server
        if (!this.renderOpts.dev) {
            this.logError(args[0]);
        }
    }
    onServerClose(listener) {
        this.cleanupListeners.add(listener);
    }
    async close() {
        await this.cleanupListeners.runAll();
    }
    getInternalWaitUntil() {
        this.internalWaitUntil ??= this.createInternalWaitUntil();
        return this.internalWaitUntil;
    }
    createInternalWaitUntil() {
        if (this.minimalMode) {
            throw new InvariantError('createInternalWaitUntil should never be called in minimal mode');
        }
        const awaiter = new AwaiterOnce({
            onError: console.error
        });
        // TODO(after): warn if the process exits before these are awaited
        this.onServerClose(()=>awaiter.awaiting());
        return awaiter.waitUntil;
    }
}

//# sourceMappingURL=next-server.js.map