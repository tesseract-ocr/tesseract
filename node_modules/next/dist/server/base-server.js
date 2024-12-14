"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    NoFallbackError: null,
    WrappedBuildError: null,
    default: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    NoFallbackError: function() {
        return NoFallbackError;
    },
    WrappedBuildError: function() {
        return WrappedBuildError;
    },
    default: function() {
        return Server;
    }
});
const _fallbackparams = require("./request/fallback-params");
const _responsecache = require("./response-cache");
const _utils = require("../shared/lib/utils");
const _url = require("url");
const _formathostname = require("./lib/format-hostname");
const _redirectstatus = require("../lib/redirect-status");
const _isedgeruntime = require("../lib/is-edge-runtime");
const _constants = require("../shared/lib/constants");
const _utils1 = require("../shared/lib/router/utils");
const _apiutils = require("./api-utils");
const _runtimeconfigexternal = require("../shared/lib/runtime-config.external");
const _revalidate = require("./lib/revalidate");
const _utils2 = require("./utils");
const _isbot = require("../shared/lib/router/utils/is-bot");
const _renderresult = /*#__PURE__*/ _interop_require_default(require("./render-result"));
const _removetrailingslash = require("../shared/lib/router/utils/remove-trailing-slash");
const _denormalizepagepath = require("../shared/lib/page-path/denormalize-page-path");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _serverutils = require("./server-utils");
const _iserror = /*#__PURE__*/ _interop_require_wildcard(require("../lib/is-error"));
const _requestmeta = require("./request-meta");
const _removepathprefix = require("../shared/lib/router/utils/remove-path-prefix");
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _gethostname = require("../shared/lib/get-hostname");
const _parseurl = require("../shared/lib/router/utils/parse-url");
const _getnextpathnameinfo = require("../shared/lib/router/utils/get-next-pathname-info");
const _approuterheaders = require("../client/components/app-router-headers");
const _localeroutenormalizer = require("./normalizers/locale-route-normalizer");
const _defaultroutematchermanager = require("./route-matcher-managers/default-route-matcher-manager");
const _apppageroutematcherprovider = require("./route-matcher-providers/app-page-route-matcher-provider");
const _approuteroutematcherprovider = require("./route-matcher-providers/app-route-route-matcher-provider");
const _pagesapiroutematcherprovider = require("./route-matcher-providers/pages-api-route-matcher-provider");
const _pagesroutematcherprovider = require("./route-matcher-providers/pages-route-matcher-provider");
const _servermanifestloader = require("./route-matcher-providers/helpers/manifest-loaders/server-manifest-loader");
const _tracer = require("./lib/trace/tracer");
const _constants1 = require("./lib/trace/constants");
const _i18nprovider = require("./lib/i18n-provider");
const _sendresponse = require("./send-response");
const _utils3 = require("./web/utils");
const _constants2 = require("../lib/constants");
const _normalizelocalepath = require("../shared/lib/i18n/normalize-locale-path");
const _nextrequest = require("./web/spec-extension/adapters/next-request");
const _matchnextdatapathname = require("./lib/match-next-data-pathname");
const _getroutefromassetpath = /*#__PURE__*/ _interop_require_default(require("../shared/lib/router/utils/get-route-from-asset-path"));
const _decodepathparams = require("./lib/router-utils/decode-path-params");
const _rsc = require("./normalizers/request/rsc");
const _stripflightheaders = require("./app-render/strip-flight-headers");
const _checks = require("./route-modules/checks");
const _prefetchrsc = require("./normalizers/request/prefetch-rsc");
const _nextdata = require("./normalizers/request/next-data");
const _serveractionrequestmeta = require("./lib/server-action-request-meta");
const _interceptionroutes = require("./lib/interception-routes");
const _toroute = require("./lib/to-route");
const _helpers = require("./base-http/helpers");
const _patchsetheader = require("./lib/patch-set-header");
const _ppr = require("./lib/experimental/ppr");
const _builtinrequestcontext = require("./after/builtin-request-context");
const _encodedTags = require("./stream-utils/encodedTags");
const _adapter = require("./web/adapter");
const _utils4 = require("./instrumentation/utils");
const _routekind = require("./route-kind");
const _fallback = require("../lib/fallback");
const _utils5 = require("./response-cache/utils");
const _scheduler = require("../lib/scheduler");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
class NoFallbackError extends Error {
}
class WrappedBuildError extends Error {
    constructor(innerError){
        super();
        this.innerError = innerError;
    }
}
class Server {
    getServerComponentsHmrCache() {
        return this.nextConfig.experimental.serverComponentsHmrCache ? globalThis.__serverComponentsHmrCache : undefined;
    }
    /**
   * This is used to persist cache scopes across
   * prefetch -> full route requests for dynamic IO
   * it's only fully used in dev
   */ constructor(options){
        var _this_nextConfig_i18n, _this_nextConfig_experimental_amp, _this_nextConfig_i18n1;
        this.handleRSCRequest = (req, _res, parsedUrl)=>{
            var _this_normalizers_prefetchRSC, _this_normalizers_rsc;
            if (!parsedUrl.pathname) return false;
            if ((_this_normalizers_prefetchRSC = this.normalizers.prefetchRSC) == null ? void 0 : _this_normalizers_prefetchRSC.match(parsedUrl.pathname)) {
                parsedUrl.pathname = this.normalizers.prefetchRSC.normalize(parsedUrl.pathname, true);
                // Mark the request as a router prefetch request.
                req.headers[_approuterheaders.RSC_HEADER.toLowerCase()] = '1';
                req.headers[_approuterheaders.NEXT_ROUTER_PREFETCH_HEADER.toLowerCase()] = '1';
                (0, _requestmeta.addRequestMeta)(req, 'isRSCRequest', true);
                (0, _requestmeta.addRequestMeta)(req, 'isPrefetchRSCRequest', true);
            } else if ((_this_normalizers_rsc = this.normalizers.rsc) == null ? void 0 : _this_normalizers_rsc.match(parsedUrl.pathname)) {
                parsedUrl.pathname = this.normalizers.rsc.normalize(parsedUrl.pathname, true);
                // Mark the request as a RSC request.
                req.headers[_approuterheaders.RSC_HEADER.toLowerCase()] = '1';
                (0, _requestmeta.addRequestMeta)(req, 'isRSCRequest', true);
            } else if (req.headers['x-now-route-matches']) {
                // If we didn't match, return with the flight headers stripped. If in
                // minimal mode we didn't match based on the path, this can't be a RSC
                // request. This is because Vercel only sends this header during
                // revalidation requests and we want the cache to instead depend on the
                // request path for flight information.
                (0, _stripflightheaders.stripFlightHeaders)(req.headers);
                return false;
            } else if (req.headers[_approuterheaders.RSC_HEADER.toLowerCase()] === '1') {
                (0, _requestmeta.addRequestMeta)(req, 'isRSCRequest', true);
                if (req.headers[_approuterheaders.NEXT_ROUTER_PREFETCH_HEADER.toLowerCase()] === '1') {
                    (0, _requestmeta.addRequestMeta)(req, 'isPrefetchRSCRequest', true);
                }
            } else {
                // Otherwise just return without doing anything.
                return false;
            }
            if (req.url) {
                const parsed = (0, _url.parse)(req.url);
                parsed.pathname = parsedUrl.pathname;
                req.url = (0, _url.format)(parsed);
            }
            return false;
        };
        this.handleNextDataRequest = async (req, res, parsedUrl)=>{
            const middleware = this.getMiddleware();
            const params = (0, _matchnextdatapathname.matchNextDataPathname)(parsedUrl.pathname);
            // ignore for non-next data URLs
            if (!params || !params.path) {
                return false;
            }
            if (params.path[0] !== this.buildId) {
                // Ignore if its a middleware request when we aren't on edge.
                if (process.env.NEXT_RUNTIME !== 'edge' && (0, _requestmeta.getRequestMeta)(req, 'middlewareInvoke')) {
                    return false;
                }
                // Make sure to 404 if the buildId isn't correct
                await this.render404(req, res, parsedUrl);
                return true;
            }
            // remove buildId from URL
            params.path.shift();
            const lastParam = params.path[params.path.length - 1];
            // show 404 if it doesn't end with .json
            if (typeof lastParam !== 'string' || !lastParam.endsWith('.json')) {
                await this.render404(req, res, parsedUrl);
                return true;
            }
            // re-create page's pathname
            let pathname = `/${params.path.join('/')}`;
            pathname = (0, _getroutefromassetpath.default)(pathname, '.json');
            // ensure trailing slash is normalized per config
            if (middleware) {
                if (this.nextConfig.trailingSlash && !pathname.endsWith('/')) {
                    pathname += '/';
                }
                if (!this.nextConfig.trailingSlash && pathname.length > 1 && pathname.endsWith('/')) {
                    pathname = pathname.substring(0, pathname.length - 1);
                }
            }
            if (this.i18nProvider) {
                var _req_headers_host;
                // Remove the port from the hostname if present.
                const hostname = req == null ? void 0 : (_req_headers_host = req.headers.host) == null ? void 0 : _req_headers_host.split(':', 1)[0].toLowerCase();
                const domainLocale = this.i18nProvider.detectDomainLocale(hostname);
                const defaultLocale = (domainLocale == null ? void 0 : domainLocale.defaultLocale) ?? this.i18nProvider.config.defaultLocale;
                const localePathResult = this.i18nProvider.analyze(pathname);
                // If the locale is detected from the path, we need to remove it
                // from the pathname.
                if (localePathResult.detectedLocale) {
                    pathname = localePathResult.pathname;
                }
                // Update the query with the detected locale and default locale.
                parsedUrl.query.__nextLocale = localePathResult.detectedLocale;
                parsedUrl.query.__nextDefaultLocale = defaultLocale;
                // If the locale is not detected from the path, we need to mark that
                // it was not inferred from default.
                if (!localePathResult.detectedLocale) {
                    delete parsedUrl.query.__nextInferredLocaleFromDefault;
                }
                // If no locale was detected and we don't have middleware, we need
                // to render a 404 page.
                if (!localePathResult.detectedLocale && !middleware) {
                    parsedUrl.query.__nextLocale = defaultLocale;
                    await this.render404(req, res, parsedUrl);
                    return true;
                }
            }
            parsedUrl.pathname = pathname;
            parsedUrl.query.__nextDataReq = '1';
            return false;
        };
        this.handleNextImageRequest = ()=>false;
        this.handleCatchallRenderRequest = ()=>false;
        this.handleCatchallMiddlewareRequest = ()=>false;
        /**
   * Normalizes a pathname without attaching any metadata from any matched
   * normalizer.
   *
   * @param pathname the pathname to normalize
   * @returns the normalized pathname
   */ this.normalize = (pathname)=>{
            const normalizers = [];
            if (this.normalizers.data) {
                normalizers.push(this.normalizers.data);
            }
            // We have to put the prefetch normalizer before the RSC normalizer
            // because the RSC normalizer will match the prefetch RSC routes too.
            if (this.normalizers.prefetchRSC) {
                normalizers.push(this.normalizers.prefetchRSC);
            }
            if (this.normalizers.rsc) {
                normalizers.push(this.normalizers.rsc);
            }
            for (const normalizer of normalizers){
                if (!normalizer.match(pathname)) continue;
                return normalizer.normalize(pathname, true);
            }
            return pathname;
        };
        this.normalizeAndAttachMetadata = async (req, res, url)=>{
            let finished = await this.handleNextImageRequest(req, res, url);
            if (finished) return true;
            if (this.enabledDirectories.pages) {
                finished = await this.handleNextDataRequest(req, res, url);
                if (finished) return true;
            }
            return false;
        };
        this.prepared = false;
        this.preparedPromise = null;
        this.customErrorNo404Warn = (0, _utils.execOnce)(()=>{
            _log.warn(`You have added a custom /_error page without a custom /404 page. This prevents the 404 page from being auto statically optimized.\nSee here for info: https://nextjs.org/docs/messages/custom-error-no-custom-404`);
        });
        const { dir = '.', quiet = false, conf, dev = false, minimalMode = false, customServer = true, hostname, port, experimentalTestProxy } = options;
        this.experimentalTestProxy = experimentalTestProxy;
        this.serverOptions = options;
        this.dir = process.env.NEXT_RUNTIME === 'edge' ? dir : require('path').resolve(dir);
        this.quiet = quiet;
        this.loadEnvConfig({
            dev
        });
        // TODO: should conf be normalized to prevent missing
        // values from causing issues as this can be user provided
        this.nextConfig = conf;
        this.hostname = hostname;
        if (this.hostname) {
            // we format the hostname so that it can be fetched
            this.fetchHostname = (0, _formathostname.formatHostname)(this.hostname);
        }
        this.port = port;
        this.distDir = process.env.NEXT_RUNTIME === 'edge' ? this.nextConfig.distDir : require('path').join(this.dir, this.nextConfig.distDir);
        this.publicDir = this.getPublicDir();
        this.hasStaticDir = !minimalMode && this.getHasStaticDir();
        this.i18nProvider = ((_this_nextConfig_i18n = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n.locales) ? new _i18nprovider.I18NProvider(this.nextConfig.i18n) : undefined;
        // Configure the locale normalizer, it's used for routes inside `pages/`.
        this.localeNormalizer = this.i18nProvider ? new _localeroutenormalizer.LocaleRouteNormalizer(this.i18nProvider) : undefined;
        // Only serverRuntimeConfig needs the default
        // publicRuntimeConfig gets it's default in client/index.js
        const { serverRuntimeConfig = {}, publicRuntimeConfig, assetPrefix, generateEtags } = this.nextConfig;
        this.buildId = this.getBuildId();
        // this is a hack to avoid Webpack knowing this is equal to this.minimalMode
        // because we replace this.minimalMode to true in production bundles.
        const minimalModeKey = 'minimalMode';
        this[minimalModeKey] = minimalMode || !!process.env.NEXT_PRIVATE_MINIMAL_MODE;
        this.enabledDirectories = this.getEnabledDirectories(dev);
        this.isAppPPREnabled = this.enabledDirectories.app && (0, _ppr.checkIsAppPPREnabled)(this.nextConfig.experimental.ppr);
        this.normalizers = {
            // We should normalize the pathname from the RSC prefix only in minimal
            // mode as otherwise that route is not exposed external to the server as
            // we instead only rely on the headers.
            rsc: this.enabledDirectories.app && this.minimalMode ? new _rsc.RSCPathnameNormalizer() : undefined,
            prefetchRSC: this.isAppPPREnabled && this.minimalMode ? new _prefetchrsc.PrefetchRSCPathnameNormalizer() : undefined,
            data: this.enabledDirectories.pages ? new _nextdata.NextDataPathnameNormalizer(this.buildId) : undefined
        };
        this.nextFontManifest = this.getNextFontManifest();
        if (process.env.NEXT_RUNTIME !== 'edge') {
            process.env.NEXT_DEPLOYMENT_ID = this.nextConfig.deploymentId || '';
        }
        this.renderOpts = {
            supportsDynamicResponse: true,
            trailingSlash: this.nextConfig.trailingSlash,
            deploymentId: this.nextConfig.deploymentId,
            strictNextHead: this.nextConfig.experimental.strictNextHead ?? true,
            poweredByHeader: this.nextConfig.poweredByHeader,
            canonicalBase: this.nextConfig.amp.canonicalBase || '',
            buildId: this.buildId,
            generateEtags,
            previewProps: this.getPrerenderManifest().preview,
            customServer: customServer === true ? true : undefined,
            ampOptimizerConfig: (_this_nextConfig_experimental_amp = this.nextConfig.experimental.amp) == null ? void 0 : _this_nextConfig_experimental_amp.optimizer,
            basePath: this.nextConfig.basePath,
            images: this.nextConfig.images,
            optimizeCss: this.nextConfig.experimental.optimizeCss,
            nextConfigOutput: this.nextConfig.output,
            nextScriptWorkers: this.nextConfig.experimental.nextScriptWorkers,
            disableOptimizedLoading: this.nextConfig.experimental.disableOptimizedLoading,
            domainLocales: (_this_nextConfig_i18n1 = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n1.domains,
            distDir: this.distDir,
            serverComponents: this.enabledDirectories.app,
            cacheLifeProfiles: this.nextConfig.experimental.cacheLife,
            enableTainting: this.nextConfig.experimental.taint,
            crossOrigin: this.nextConfig.crossOrigin ? this.nextConfig.crossOrigin : undefined,
            largePageDataBytes: this.nextConfig.experimental.largePageDataBytes,
            // Only the `publicRuntimeConfig` key is exposed to the client side
            // It'll be rendered as part of __NEXT_DATA__ on the client side
            runtimeConfig: Object.keys(publicRuntimeConfig).length > 0 ? publicRuntimeConfig : undefined,
            // @ts-expect-error internal field not publicly exposed
            isExperimentalCompile: this.nextConfig.experimental.isExperimentalCompile,
            experimental: {
                expireTime: this.nextConfig.expireTime,
                clientTraceMetadata: this.nextConfig.experimental.clientTraceMetadata,
                dynamicIO: this.nextConfig.experimental.dynamicIO ?? false,
                inlineCss: this.nextConfig.experimental.inlineCss ?? false,
                authInterrupts: !!this.nextConfig.experimental.authInterrupts
            },
            onInstrumentationRequestError: this.instrumentationOnRequestError.bind(this),
            reactMaxHeadersLength: this.nextConfig.reactMaxHeadersLength
        };
        // Initialize next/config with the environment configuration
        (0, _runtimeconfigexternal.setConfig)({
            serverRuntimeConfig,
            publicRuntimeConfig
        });
        this.pagesManifest = this.getPagesManifest();
        this.appPathsManifest = this.getAppPathsManifest();
        this.appPathRoutes = this.getAppPathRoutes();
        this.interceptionRoutePatterns = this.getinterceptionRoutePatterns();
        // Configure the routes.
        this.matchers = this.getRouteMatchers();
        // Start route compilation. We don't wait for the routes to finish loading
        // because we use the `waitTillReady` promise below in `handleRequest` to
        // wait. Also we can't `await` in the constructor.
        void this.matchers.reload();
        this.setAssetPrefix(assetPrefix);
        this.responseCache = this.getResponseCache({
            dev
        });
    }
    reloadMatchers() {
        return this.matchers.reload();
    }
    getRouteMatchers() {
        // Create a new manifest loader that get's the manifests from the server.
        const manifestLoader = new _servermanifestloader.ServerManifestLoader((name)=>{
            switch(name){
                case _constants.PAGES_MANIFEST:
                    return this.getPagesManifest() ?? null;
                case _constants.APP_PATHS_MANIFEST:
                    return this.getAppPathsManifest() ?? null;
                default:
                    return null;
            }
        });
        // Configure the matchers and handlers.
        const matchers = new _defaultroutematchermanager.DefaultRouteMatcherManager();
        // Match pages under `pages/`.
        matchers.push(new _pagesroutematcherprovider.PagesRouteMatcherProvider(this.distDir, manifestLoader, this.i18nProvider));
        // Match api routes under `pages/api/`.
        matchers.push(new _pagesapiroutematcherprovider.PagesAPIRouteMatcherProvider(this.distDir, manifestLoader, this.i18nProvider));
        // If the app directory is enabled, then add the app matchers and handlers.
        if (this.enabledDirectories.app) {
            // Match app pages under `app/`.
            matchers.push(new _apppageroutematcherprovider.AppPageRouteMatcherProvider(this.distDir, manifestLoader));
            matchers.push(new _approuteroutematcherprovider.AppRouteRouteMatcherProvider(this.distDir, manifestLoader));
        }
        return matchers;
    }
    async instrumentationOnRequestError(...args) {
        const [err, req, ctx] = args;
        if (this.instrumentation) {
            try {
                await (this.instrumentation.onRequestError == null ? void 0 : this.instrumentation.onRequestError.call(this.instrumentation, err, {
                    path: req.url || '',
                    method: req.method || 'GET',
                    // Normalize middleware headers and other server request headers
                    headers: req instanceof _adapter.NextRequestHint ? Object.fromEntries(req.headers.entries()) : req.headers
                }, ctx));
            } catch (handlerErr) {
                // Log the soft error and continue, since errors can thrown from react stream handler
                console.error('Error in instrumentation.onRequestError:', handlerErr);
            }
        }
    }
    logError(err) {
        if (this.quiet) return;
        _log.error(err);
    }
    async handleRequest(req, res, parsedUrl) {
        await this.prepare();
        const method = req.method.toUpperCase();
        const tracer = (0, _tracer.getTracer)();
        return tracer.withPropagatedContext(req.headers, ()=>{
            return tracer.trace(_constants1.BaseServerSpan.handleRequest, {
                spanName: `${method} ${req.url}`,
                kind: _tracer.SpanKind.SERVER,
                attributes: {
                    'http.method': method,
                    'http.target': req.url
                }
            }, async (span)=>this.handleRequestImpl(req, res, parsedUrl).finally(()=>{
                    if (!span) return;
                    const isRSCRequest = (0, _requestmeta.getRequestMeta)(req, 'isRSCRequest') ?? false;
                    span.setAttributes({
                        'http.status_code': res.statusCode,
                        'next.rsc': isRSCRequest
                    });
                    const rootSpanAttributes = tracer.getRootSpanAttributes();
                    // We were unable to get attributes, probably OTEL is not enabled
                    if (!rootSpanAttributes) return;
                    if (rootSpanAttributes.get('next.span_type') !== _constants1.BaseServerSpan.handleRequest) {
                        console.warn(`Unexpected root span type '${rootSpanAttributes.get('next.span_type')}'. Please report this Next.js issue https://github.com/vercel/next.js`);
                        return;
                    }
                    const route = rootSpanAttributes.get('next.route');
                    if (route) {
                        const name = isRSCRequest ? `RSC ${method} ${route}` : `${method} ${route}`;
                        span.setAttributes({
                            'next.route': route,
                            'http.route': route,
                            'next.span_name': name
                        });
                        span.updateName(name);
                    } else {
                        span.updateName(isRSCRequest ? `RSC ${method} ${req.url}` : `${method} ${req.url}`);
                    }
                }));
        });
    }
    async handleRequestImpl(req, res, parsedUrl) {
        try {
            var _originalRequest_socket, _originalRequest_socket1, _this_i18nProvider, _this_i18nProvider1, _this_nextConfig_i18n;
            // Wait for the matchers to be ready.
            await this.matchers.waitTillReady();
            // ensure cookies set in middleware are merged and
            // not overridden by API routes/getServerSideProps
            (0, _patchsetheader.patchSetHeaderWithCookieSupport)(req, (0, _helpers.isNodeNextResponse)(res) ? res.originalResponse : res);
            const urlParts = (req.url || '').split('?', 1);
            const urlNoQuery = urlParts[0];
            // this normalizes repeated slashes in the path e.g. hello//world ->
            // hello/world or backslashes to forward slashes, this does not
            // handle trailing slash as that is handled the same as a next.config.js
            // redirect
            if (urlNoQuery == null ? void 0 : urlNoQuery.match(/(\\|\/\/)/)) {
                const cleanUrl = (0, _utils.normalizeRepeatedSlashes)(req.url);
                res.redirect(cleanUrl, 308).body(cleanUrl).send();
                return;
            }
            // Parse url if parsedUrl not provided
            if (!parsedUrl || typeof parsedUrl !== 'object') {
                if (!req.url) {
                    throw new Error('Invariant: url can not be undefined');
                }
                parsedUrl = (0, _url.parse)(req.url, true);
            }
            if (!parsedUrl.pathname) {
                throw new Error("Invariant: pathname can't be empty");
            }
            // Parse the querystring ourselves if the user doesn't handle querystring parsing
            if (typeof parsedUrl.query === 'string') {
                parsedUrl.query = Object.fromEntries(new URLSearchParams(parsedUrl.query));
            }
            // Update the `x-forwarded-*` headers.
            const { originalRequest = null } = (0, _helpers.isNodeNextRequest)(req) ? req : {};
            const xForwardedProto = originalRequest == null ? void 0 : originalRequest.headers['x-forwarded-proto'];
            const isHttps = xForwardedProto ? xForwardedProto === 'https' : !!(originalRequest == null ? void 0 : (_originalRequest_socket = originalRequest.socket) == null ? void 0 : _originalRequest_socket.encrypted);
            req.headers['x-forwarded-host'] ??= req.headers['host'] ?? this.hostname;
            req.headers['x-forwarded-port'] ??= this.port ? this.port.toString() : isHttps ? '443' : '80';
            req.headers['x-forwarded-proto'] ??= isHttps ? 'https' : 'http';
            req.headers['x-forwarded-for'] ??= originalRequest == null ? void 0 : (_originalRequest_socket1 = originalRequest.socket) == null ? void 0 : _originalRequest_socket1.remoteAddress;
            // Validate that if i18n isn't configured or the passed parameters are not
            // valid it should be removed from the query.
            if (!((_this_i18nProvider = this.i18nProvider) == null ? void 0 : _this_i18nProvider.validateQuery(parsedUrl.query))) {
                delete parsedUrl.query.__nextLocale;
                delete parsedUrl.query.__nextDefaultLocale;
                delete parsedUrl.query.__nextInferredLocaleFromDefault;
            }
            // This should be done before any normalization of the pathname happens as
            // it captures the initial URL.
            this.attachRequestMeta(req, parsedUrl);
            let finished = await this.handleRSCRequest(req, res, parsedUrl);
            if (finished) return;
            const domainLocale = (_this_i18nProvider1 = this.i18nProvider) == null ? void 0 : _this_i18nProvider1.detectDomainLocale((0, _gethostname.getHostname)(parsedUrl, req.headers));
            const defaultLocale = (domainLocale == null ? void 0 : domainLocale.defaultLocale) || ((_this_nextConfig_i18n = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n.defaultLocale);
            parsedUrl.query.__nextDefaultLocale = defaultLocale;
            const url = (0, _parseurl.parseUrl)(req.url.replace(/^\/+/, '/'));
            const pathnameInfo = (0, _getnextpathnameinfo.getNextPathnameInfo)(url.pathname, {
                nextConfig: this.nextConfig,
                i18nProvider: this.i18nProvider
            });
            url.pathname = pathnameInfo.pathname;
            if (pathnameInfo.basePath) {
                req.url = (0, _removepathprefix.removePathPrefix)(req.url, this.nextConfig.basePath);
            }
            const useMatchedPathHeader = this.minimalMode && typeof req.headers[_constants2.MATCHED_PATH_HEADER] === 'string';
            // TODO: merge handling with invokePath
            if (useMatchedPathHeader) {
                try {
                    var _this_normalizers_data, _this_i18nProvider2, _this_getRoutesManifest;
                    if (this.enabledDirectories.app) {
                        // ensure /index path is normalized for prerender
                        // in minimal mode
                        if (req.url.match(/^\/index($|\?)/)) {
                            req.url = req.url.replace(/^\/index/, '/');
                        }
                        parsedUrl.pathname = parsedUrl.pathname === '/index' ? '/' : parsedUrl.pathname;
                    }
                    // x-matched-path is the source of truth, it tells what page
                    // should be rendered because we don't process rewrites in minimalMode
                    let { pathname: matchedPath } = new URL(req.headers[_constants2.MATCHED_PATH_HEADER], 'http://localhost');
                    let { pathname: urlPathname } = new URL(req.url, 'http://localhost');
                    // For ISR the URL is normalized to the prerenderPath so if
                    // it's a data request the URL path will be the data URL,
                    // basePath is already stripped by this point
                    if ((_this_normalizers_data = this.normalizers.data) == null ? void 0 : _this_normalizers_data.match(urlPathname)) {
                        parsedUrl.query.__nextDataReq = '1';
                    } else if (this.isAppPPREnabled && this.minimalMode && req.headers[_constants2.NEXT_RESUME_HEADER] === '1' && req.method === 'POST') {
                        // Decode the postponed state from the request body, it will come as
                        // an array of buffers, so collect them and then concat them to form
                        // the string.
                        const body = [];
                        for await (const chunk of req.body){
                            body.push(chunk);
                        }
                        const postponed = Buffer.concat(body).toString('utf8');
                        (0, _requestmeta.addRequestMeta)(req, 'postponed', postponed);
                    }
                    matchedPath = this.normalize(matchedPath);
                    const normalizedUrlPath = this.stripNextDataPath(urlPathname);
                    // Perform locale detection and normalization.
                    const localeAnalysisResult = (_this_i18nProvider2 = this.i18nProvider) == null ? void 0 : _this_i18nProvider2.analyze(matchedPath, {
                        defaultLocale
                    });
                    // The locale result will be defined even if the locale was not
                    // detected for the request because it will be inferred from the
                    // default locale.
                    if (localeAnalysisResult) {
                        parsedUrl.query.__nextLocale = localeAnalysisResult.detectedLocale;
                        // If the detected locale was inferred from the default locale, we
                        // need to modify the metadata on the request to indicate that.
                        if (localeAnalysisResult.inferredFromDefault) {
                            parsedUrl.query.__nextInferredLocaleFromDefault = '1';
                        } else {
                            delete parsedUrl.query.__nextInferredLocaleFromDefault;
                        }
                    }
                    // TODO: check if this is needed any more?
                    matchedPath = (0, _denormalizepagepath.denormalizePagePath)(matchedPath);
                    let srcPathname = matchedPath;
                    let pageIsDynamic = (0, _utils1.isDynamicRoute)(srcPathname);
                    if (!pageIsDynamic) {
                        const match = await this.matchers.match(srcPathname, {
                            i18n: localeAnalysisResult
                        });
                        // Update the source pathname to the matched page's pathname.
                        if (match) {
                            srcPathname = match.definition.pathname;
                            // The page is dynamic if the params are defined.
                            pageIsDynamic = typeof match.params !== 'undefined';
                        }
                    }
                    // The rest of this function can't handle i18n properly, so ensure we
                    // restore the pathname with the locale information stripped from it
                    // now that we're done matching if we're using i18n.
                    if (localeAnalysisResult) {
                        matchedPath = localeAnalysisResult.pathname;
                    }
                    const utils = (0, _serverutils.getUtils)({
                        pageIsDynamic,
                        page: srcPathname,
                        i18n: this.nextConfig.i18n,
                        basePath: this.nextConfig.basePath,
                        rewrites: ((_this_getRoutesManifest = this.getRoutesManifest()) == null ? void 0 : _this_getRoutesManifest.rewrites) || {
                            beforeFiles: [],
                            afterFiles: [],
                            fallback: []
                        },
                        caseSensitive: !!this.nextConfig.experimental.caseSensitiveRoutes
                    });
                    // Ensure parsedUrl.pathname includes locale before processing
                    // rewrites or they won't match correctly.
                    if (defaultLocale && !pathnameInfo.locale) {
                        parsedUrl.pathname = `/${defaultLocale}${parsedUrl.pathname}`;
                    }
                    const pathnameBeforeRewrite = parsedUrl.pathname;
                    const rewriteParams = utils.handleRewrites(req, parsedUrl);
                    const rewriteParamKeys = Object.keys(rewriteParams);
                    const didRewrite = pathnameBeforeRewrite !== parsedUrl.pathname;
                    if (didRewrite && parsedUrl.pathname) {
                        (0, _requestmeta.addRequestMeta)(req, 'rewroteURL', parsedUrl.pathname);
                    }
                    const routeParamKeys = new Set();
                    for (const key of Object.keys(parsedUrl.query)){
                        const value = parsedUrl.query[key];
                        (0, _utils3.normalizeNextQueryParam)(key, (normalizedKey)=>{
                            if (!parsedUrl) return; // typeguard
                            parsedUrl.query[normalizedKey] = value;
                            routeParamKeys.add(normalizedKey);
                            delete parsedUrl.query[key];
                        });
                    }
                    // interpolate dynamic params and normalize URL if needed
                    if (pageIsDynamic) {
                        let params = {};
                        let paramsResult = utils.normalizeDynamicRouteParams(parsedUrl.query);
                        // for prerendered ISR paths we attempt parsing the route
                        // params from the URL directly as route-matches may not
                        // contain the correct values due to the filesystem path
                        // matching before the dynamic route has been matched
                        if (!paramsResult.hasValidParams && !(0, _utils1.isDynamicRoute)(normalizedUrlPath)) {
                            let matcherParams = utils.dynamicRouteMatcher == null ? void 0 : utils.dynamicRouteMatcher.call(utils, normalizedUrlPath);
                            if (matcherParams) {
                                utils.normalizeDynamicRouteParams(matcherParams);
                                Object.assign(paramsResult.params, matcherParams);
                                paramsResult.hasValidParams = true;
                            }
                        }
                        // if an action request is bypassing a prerender and we
                        // don't have the params in the URL since it was prerendered
                        // and matched during handle: 'filesystem' rather than dynamic route
                        // resolving we need to parse the params from the matched-path.
                        // Note: this is similar to above case but from match-path instead
                        // of from the request URL since a rewrite could cause that to not
                        // match the src pathname
                        if (// we can have a collision with /index and a top-level /[slug]
                        matchedPath !== '/index' && !paramsResult.hasValidParams && !(0, _utils1.isDynamicRoute)(matchedPath)) {
                            let matcherParams = utils.dynamicRouteMatcher == null ? void 0 : utils.dynamicRouteMatcher.call(utils, matchedPath);
                            if (matcherParams) {
                                const curParamsResult = utils.normalizeDynamicRouteParams(matcherParams);
                                if (curParamsResult.hasValidParams) {
                                    Object.assign(params, matcherParams);
                                    paramsResult = curParamsResult;
                                }
                            }
                        }
                        if (paramsResult.hasValidParams) {
                            params = paramsResult.params;
                        }
                        if (req.headers['x-now-route-matches'] && (0, _utils1.isDynamicRoute)(matchedPath) && !paramsResult.hasValidParams) {
                            const opts = {};
                            const routeParams = utils.getParamsFromRouteMatches(req, opts, parsedUrl.query.__nextLocale || '');
                            // If this returns a locale, it means that the locale was detected
                            // from the pathname.
                            if (opts.locale) {
                                parsedUrl.query.__nextLocale = opts.locale;
                                // As the locale was parsed from the pathname, we should mark
                                // that the locale was not inferred as the default.
                                delete parsedUrl.query.__nextInferredLocaleFromDefault;
                            }
                            paramsResult = utils.normalizeDynamicRouteParams(routeParams, true);
                            if (paramsResult.hasValidParams) {
                                params = paramsResult.params;
                            }
                        }
                        // handle the actual dynamic route name being requested
                        if (utils.defaultRouteMatches && normalizedUrlPath === srcPathname && !paramsResult.hasValidParams && !utils.normalizeDynamicRouteParams({
                            ...params
                        }, true).hasValidParams) {
                            params = utils.defaultRouteMatches;
                            // Mark that the default route matches were set on the request
                            // during routing.
                            (0, _requestmeta.addRequestMeta)(req, 'didSetDefaultRouteMatches', true);
                        }
                        if (params) {
                            matchedPath = utils.interpolateDynamicPath(srcPathname, params);
                            req.url = utils.interpolateDynamicPath(req.url, params);
                        }
                    }
                    if (pageIsDynamic || didRewrite) {
                        var _utils_defaultRouteRegex;
                        utils.normalizeVercelUrl(req, true, [
                            ...rewriteParamKeys,
                            ...Object.keys(((_utils_defaultRouteRegex = utils.defaultRouteRegex) == null ? void 0 : _utils_defaultRouteRegex.groups) || {})
                        ]);
                    }
                    for (const key of routeParamKeys){
                        delete parsedUrl.query[key];
                    }
                    parsedUrl.pathname = matchedPath;
                    url.pathname = parsedUrl.pathname;
                    finished = await this.normalizeAndAttachMetadata(req, res, parsedUrl);
                    if (finished) return;
                } catch (err) {
                    if (err instanceof _utils.DecodeError || err instanceof _utils.NormalizeError) {
                        res.statusCode = 400;
                        return this.renderError(null, req, res, '/_error', {});
                    }
                    throw err;
                }
            }
            (0, _requestmeta.addRequestMeta)(req, 'isLocaleDomain', Boolean(domainLocale));
            if (pathnameInfo.locale) {
                req.url = (0, _url.format)(url);
                (0, _requestmeta.addRequestMeta)(req, 'didStripLocale', true);
            }
            // If we aren't in minimal mode or there is no locale in the query
            // string, add the locale to the query string.
            if (!this.minimalMode || !parsedUrl.query.__nextLocale) {
                // If the locale is in the pathname, add it to the query string.
                if (pathnameInfo.locale) {
                    parsedUrl.query.__nextLocale = pathnameInfo.locale;
                } else if (defaultLocale) {
                    parsedUrl.query.__nextLocale = defaultLocale;
                    parsedUrl.query.__nextInferredLocaleFromDefault = '1';
                }
            }
            // set incremental cache to request meta so it can
            // be passed down for edge functions and the fetch disk
            // cache can be leveraged locally
            if (!this.serverOptions.webServerConfig && !(0, _requestmeta.getRequestMeta)(req, 'incrementalCache')) {
                let protocol = 'https:';
                try {
                    const parsedFullUrl = new URL((0, _requestmeta.getRequestMeta)(req, 'initURL') || '/', 'http://n');
                    protocol = parsedFullUrl.protocol;
                } catch  {}
                const incrementalCache = await this.getIncrementalCache({
                    requestHeaders: Object.assign({}, req.headers),
                    requestProtocol: protocol.substring(0, protocol.length - 1)
                });
                const _globalThis = globalThis;
                if (_globalThis.__nextCacheHandlers) {
                    var _req_headers_NEXT_CACHE_REVALIDATED_TAGS_HEADER;
                    const expiredTags = ((_req_headers_NEXT_CACHE_REVALIDATED_TAGS_HEADER = req.headers[_constants2.NEXT_CACHE_REVALIDATED_TAGS_HEADER]) == null ? void 0 : _req_headers_NEXT_CACHE_REVALIDATED_TAGS_HEADER.split(',')) || [];
                    for (const handler of Object.values(_globalThis.__nextCacheHandlers)){
                        if (typeof handler.receiveExpiredTags === 'function') {
                            await handler.receiveExpiredTags(...expiredTags);
                        }
                    }
                }
                incrementalCache.resetRequestCache();
                (0, _requestmeta.addRequestMeta)(req, 'incrementalCache', incrementalCache);
                globalThis.__incrementalCache = incrementalCache;
            }
            // set server components HMR cache to request meta so it can be passed
            // down for edge functions
            if (!(0, _requestmeta.getRequestMeta)(req, 'serverComponentsHmrCache')) {
                (0, _requestmeta.addRequestMeta)(req, 'serverComponentsHmrCache', this.getServerComponentsHmrCache());
            }
            // when invokePath is specified we can short short circuit resolving
            // we only honor this header if we are inside of a render worker to
            // prevent external users coercing the routing path
            const invokePath = (0, _requestmeta.getRequestMeta)(req, 'invokePath');
            const useInvokePath = !useMatchedPathHeader && process.env.NEXT_RUNTIME !== 'edge' && invokePath;
            if (useInvokePath) {
                var _this_nextConfig_i18n1;
                const invokeStatus = (0, _requestmeta.getRequestMeta)(req, 'invokeStatus');
                if (invokeStatus) {
                    const invokeQuery = (0, _requestmeta.getRequestMeta)(req, 'invokeQuery');
                    if (invokeQuery) {
                        Object.assign(parsedUrl.query, invokeQuery);
                    }
                    res.statusCode = invokeStatus;
                    let err = (0, _requestmeta.getRequestMeta)(req, 'invokeError') || null;
                    return this.renderError(err, req, res, '/_error', parsedUrl.query);
                }
                const parsedMatchedPath = new URL(invokePath || '/', 'http://n');
                const invokePathnameInfo = (0, _getnextpathnameinfo.getNextPathnameInfo)(parsedMatchedPath.pathname, {
                    nextConfig: this.nextConfig,
                    parseData: false
                });
                if (invokePathnameInfo.locale) {
                    parsedUrl.query.__nextLocale = invokePathnameInfo.locale;
                }
                if (parsedUrl.pathname !== parsedMatchedPath.pathname) {
                    parsedUrl.pathname = parsedMatchedPath.pathname;
                    (0, _requestmeta.addRequestMeta)(req, 'rewroteURL', invokePathnameInfo.pathname);
                }
                const normalizeResult = (0, _normalizelocalepath.normalizeLocalePath)((0, _removepathprefix.removePathPrefix)(parsedUrl.pathname, this.nextConfig.basePath || ''), ((_this_nextConfig_i18n1 = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n1.locales) || []);
                if (normalizeResult.detectedLocale) {
                    parsedUrl.query.__nextLocale = normalizeResult.detectedLocale;
                }
                parsedUrl.pathname = normalizeResult.pathname;
                for (const key of Object.keys(parsedUrl.query)){
                    if (!key.startsWith('__next') && !key.startsWith('_next')) {
                        delete parsedUrl.query[key];
                    }
                }
                const invokeQuery = (0, _requestmeta.getRequestMeta)(req, 'invokeQuery');
                if (invokeQuery) {
                    Object.assign(parsedUrl.query, invokeQuery);
                }
                finished = await this.normalizeAndAttachMetadata(req, res, parsedUrl);
                if (finished) return;
                await this.handleCatchallRenderRequest(req, res, parsedUrl);
                return;
            }
            if (process.env.NEXT_RUNTIME !== 'edge' && (0, _requestmeta.getRequestMeta)(req, 'middlewareInvoke')) {
                finished = await this.normalizeAndAttachMetadata(req, res, parsedUrl);
                if (finished) return;
                finished = await this.handleCatchallMiddlewareRequest(req, res, parsedUrl);
                if (finished) return;
                const err = new Error();
                err.result = {
                    response: new Response(null, {
                        headers: {
                            'x-middleware-next': '1'
                        }
                    })
                };
                err.bubble = true;
                throw err;
            }
            // This wasn't a request via the matched path or the invoke path, so
            // prepare for a legacy run by removing the base path.
            // ensure we strip the basePath when not using an invoke header
            if (!useMatchedPathHeader && pathnameInfo.basePath) {
                parsedUrl.pathname = (0, _removepathprefix.removePathPrefix)(parsedUrl.pathname, pathnameInfo.basePath);
            }
            res.statusCode = 200;
            return await this.run(req, res, parsedUrl);
        } catch (err) {
            if (err instanceof NoFallbackError) {
                throw err;
            }
            if (err && typeof err === 'object' && err.code === 'ERR_INVALID_URL' || err instanceof _utils.DecodeError || err instanceof _utils.NormalizeError) {
                res.statusCode = 400;
                return this.renderError(null, req, res, '/_error', {});
            }
            if (this.minimalMode || this.renderOpts.dev || (0, _tracer.isBubbledError)(err) && err.bubble) {
                throw err;
            }
            this.logError((0, _iserror.getProperError)(err));
            res.statusCode = 500;
            res.body('Internal Server Error').send();
        }
    }
    /**
   * @internal - this method is internal to Next.js and should not be used directly by end-users
   */ getRequestHandlerWithMetadata(meta) {
        const handler = this.getRequestHandler();
        return (req, res, parsedUrl)=>{
            (0, _requestmeta.setRequestMeta)(req, meta);
            return handler(req, res, parsedUrl);
        };
    }
    getRequestHandler() {
        return this.handleRequest.bind(this);
    }
    setAssetPrefix(prefix) {
        this.renderOpts.assetPrefix = prefix ? prefix.replace(/\/$/, '') : '';
    }
    /**
   * Runs async initialization of server.
   * It is idempotent, won't fire underlying initialization more than once.
   */ async prepare() {
        if (this.prepared) return;
        if (this.preparedPromise === null) {
            // Get instrumentation module
            this.instrumentation = await this.loadInstrumentationModule();
            this.preparedPromise = this.prepareImpl().then(()=>{
                this.prepared = true;
                this.preparedPromise = null;
            });
        }
        return this.preparedPromise;
    }
    async prepareImpl() {}
    async loadInstrumentationModule() {}
    async close() {}
    getAppPathRoutes() {
        const appPathRoutes = {};
        Object.keys(this.appPathsManifest || {}).forEach((entry)=>{
            const normalizedPath = (0, _apppaths.normalizeAppPath)(entry);
            if (!appPathRoutes[normalizedPath]) {
                appPathRoutes[normalizedPath] = [];
            }
            appPathRoutes[normalizedPath].push(entry);
        });
        return appPathRoutes;
    }
    async run(req, res, parsedUrl) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.run, async ()=>this.runImpl(req, res, parsedUrl));
    }
    async runImpl(req, res, parsedUrl) {
        await this.handleCatchallRenderRequest(req, res, parsedUrl);
    }
    async pipe(fn, partialContext) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.pipe, async ()=>this.pipeImpl(fn, partialContext));
    }
    async pipeImpl(fn, partialContext) {
        const isBotRequest = (0, _isbot.isBot)(partialContext.req.headers['user-agent'] || '');
        const ctx = {
            ...partialContext,
            renderOpts: {
                ...this.renderOpts,
                supportsDynamicResponse: !isBotRequest,
                isBot: !!isBotRequest
            }
        };
        const payload = await fn(ctx);
        if (payload === null) {
            return;
        }
        const { req, res } = ctx;
        const originalStatus = res.statusCode;
        const { body, type } = payload;
        let { revalidate } = payload;
        if (!res.sent) {
            const { generateEtags, poweredByHeader, dev } = this.renderOpts;
            // In dev, we should not cache pages for any reason.
            if (dev) {
                res.setHeader('Cache-Control', 'no-store, must-revalidate');
                revalidate = undefined;
            }
            await this.sendRenderResult(req, res, {
                result: body,
                type,
                generateEtags,
                poweredByHeader,
                revalidate,
                expireTime: this.nextConfig.expireTime
            });
            res.statusCode = originalStatus;
        }
    }
    async getStaticHTML(fn, partialContext) {
        const ctx = {
            ...partialContext,
            renderOpts: {
                ...this.renderOpts,
                supportsDynamicResponse: false
            }
        };
        const payload = await fn(ctx);
        if (payload === null) {
            return null;
        }
        return payload.body.toUnchunkedString();
    }
    async render(req, res, pathname, query = {}, parsedUrl, internalRender = false) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.render, async ()=>this.renderImpl(req, res, pathname, query, parsedUrl, internalRender));
    }
    getWaitUntil() {
        const builtinRequestContext = (0, _builtinrequestcontext.getBuiltinRequestContext)();
        if (builtinRequestContext) {
            // the platform provided a request context.
            // use the `waitUntil` from there, whether actually present or not --
            // if not present, `after` will error.
            // NOTE: if we're in an edge runtime sandbox, this context will be used to forward the outer waitUntil.
            return builtinRequestContext.waitUntil;
        }
        if (this.minimalMode) {
            // we're built for a serverless environment, and `waitUntil` is not available,
            // but using a noop would likely lead to incorrect behavior,
            // because we have no way of keeping the invocation alive.
            // return nothing, and `after` will error if used.
            //
            // NOTE: for edge functions, `NextWebServer` always runs in minimal mode.
            //
            // NOTE: if we're in an edge runtime sandbox, waitUntil will be passed in using "@next/request-context",
            // so we won't get here.
            return undefined;
        }
        return this.getInternalWaitUntil();
    }
    getInternalWaitUntil() {
        return undefined;
    }
    async renderImpl(req, res, pathname, query = {}, parsedUrl, internalRender = false) {
        var _req_url;
        if (!pathname.startsWith('/')) {
            console.warn(`Cannot render page with path "${pathname}", did you mean "/${pathname}"?. See more info here: https://nextjs.org/docs/messages/render-no-starting-slash`);
        }
        if (this.renderOpts.customServer && pathname === '/index' && !await this.hasPage('/index')) {
            // maintain backwards compatibility for custom server
            // (see custom-server integration tests)
            pathname = '/';
        }
        // we allow custom servers to call render for all URLs
        // so check if we need to serve a static _next file or not.
        // we don't modify the URL for _next/data request but still
        // call render so we special case this to prevent an infinite loop
        if (!internalRender && !this.minimalMode && !query.__nextDataReq && (((_req_url = req.url) == null ? void 0 : _req_url.match(/^\/_next\//)) || this.hasStaticDir && req.url.match(/^\/static\//))) {
            return this.handleRequest(req, res, parsedUrl);
        }
        if ((0, _utils2.isBlockedPage)(pathname)) {
            return this.render404(req, res, parsedUrl);
        }
        return this.pipe((ctx)=>this.renderToResponse(ctx), {
            req,
            res,
            pathname,
            query
        });
    }
    async getStaticPaths({ pathname }) {
        var _this_getPrerenderManifest_dynamicRoutes_pathname;
        // Read whether or not fallback should exist from the manifest.
        const fallbackField = (_this_getPrerenderManifest_dynamicRoutes_pathname = this.getPrerenderManifest().dynamicRoutes[pathname]) == null ? void 0 : _this_getPrerenderManifest_dynamicRoutes_pathname.fallback;
        return {
            // `staticPaths` is intentionally set to `undefined` as it should've
            // been caught when checking disk data.
            staticPaths: undefined,
            fallbackMode: (0, _fallback.parseFallbackField)(fallbackField)
        };
    }
    async renderToResponseWithComponents(requestContext, findComponentsResult) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.renderToResponseWithComponents, async ()=>this.renderToResponseWithComponentsImpl(requestContext, findComponentsResult));
    }
    pathCouldBeIntercepted(resolvedPathname) {
        return (0, _interceptionroutes.isInterceptionRouteAppPath)(resolvedPathname) || this.interceptionRoutePatterns.some((regexp)=>{
            return regexp.test(resolvedPathname);
        });
    }
    setVaryHeader(req, res, isAppPath, resolvedPathname) {
        const baseVaryHeader = `${_approuterheaders.RSC_HEADER}, ${_approuterheaders.NEXT_ROUTER_STATE_TREE_HEADER}, ${_approuterheaders.NEXT_ROUTER_PREFETCH_HEADER}, ${_approuterheaders.NEXT_ROUTER_SEGMENT_PREFETCH_HEADER}`;
        const isRSCRequest = (0, _requestmeta.getRequestMeta)(req, 'isRSCRequest') ?? false;
        let addedNextUrlToVary = false;
        if (isAppPath && this.pathCouldBeIntercepted(resolvedPathname)) {
            // Interception route responses can vary based on the `Next-URL` header.
            // We use the Vary header to signal this behavior to the client to properly cache the response.
            res.setHeader('vary', `${baseVaryHeader}, ${_approuterheaders.NEXT_URL}`);
            addedNextUrlToVary = true;
        } else if (isAppPath || isRSCRequest) {
            // We don't need to include `Next-URL` in the Vary header for non-interception routes since it won't affect the response.
            // We also set this header for pages to avoid caching issues when navigating between pages and app.
            res.setHeader('vary', baseVaryHeader);
        }
        if (!addedNextUrlToVary) {
            // Remove `Next-URL` from the request headers we determined it wasn't necessary to include in the Vary header.
            // This is to avoid any dependency on the `Next-URL` header being present when preparing the response.
            delete req.headers[_approuterheaders.NEXT_URL];
        }
    }
    async renderToResponseWithComponentsImpl({ req, res, pathname, renderOpts: opts }, { components, query }) {
        var _components_Component, _this, _this_nextConfig_i18n, _this_nextConfig_i18n1, _cacheEntry_value, _cacheEntry_value1;
        if (pathname === _constants.UNDERSCORE_NOT_FOUND_ROUTE) {
            pathname = '/404';
        }
        const isErrorPathname = pathname === '/_error';
        const is404Page = pathname === '/404' || isErrorPathname && res.statusCode === 404;
        const is500Page = pathname === '/500' || isErrorPathname && res.statusCode === 500;
        const isAppPath = components.isAppPath === true;
        const hasServerProps = !!components.getServerSideProps;
        let hasGetStaticPaths = !!components.getStaticPaths;
        const isServerAction = (0, _serveractionrequestmeta.getIsServerAction)(req);
        const hasGetInitialProps = !!((_components_Component = components.Component) == null ? void 0 : _components_Component.getInitialProps);
        let isSSG = !!components.getStaticProps;
        // Compute the iSSG cache key. We use the rewroteUrl since
        // pages with fallback: false are allowed to be rewritten to
        // and we need to look up the path by the rewritten path
        let urlPathname = (0, _url.parse)(req.url || '').pathname || '/';
        let resolvedUrlPathname = (0, _requestmeta.getRequestMeta)(req, 'rewroteURL') || urlPathname;
        this.setVaryHeader(req, res, isAppPath, resolvedUrlPathname);
        let staticPaths;
        let fallbackMode;
        let hasFallback = false;
        const isDynamic = (0, _utils1.isDynamicRoute)(components.page);
        const prerenderManifest = this.getPrerenderManifest();
        if (isAppPath && isDynamic) {
            const pathsResult = await this.getStaticPaths({
                pathname,
                page: components.page,
                isAppPath,
                requestHeaders: req.headers
            });
            staticPaths = pathsResult.staticPaths;
            fallbackMode = pathsResult.fallbackMode;
            hasFallback = typeof fallbackMode !== 'undefined';
            if (this.nextConfig.output === 'export') {
                const page = components.page;
                if (!staticPaths) {
                    throw new Error(`Page "${page}" is missing exported function "generateStaticParams()", which is required with "output: export" config.`);
                }
                const resolvedWithoutSlash = (0, _removetrailingslash.removeTrailingSlash)(resolvedUrlPathname);
                if (!staticPaths.includes(resolvedWithoutSlash)) {
                    throw new Error(`Page "${page}" is missing param "${resolvedWithoutSlash}" in "generateStaticParams()", which is required with "output: export" config.`);
                }
            }
            if (hasFallback) {
                hasGetStaticPaths = true;
            }
        }
        if (hasFallback || (staticPaths == null ? void 0 : staticPaths.includes(resolvedUrlPathname)) || // this signals revalidation in deploy environments
        // TODO: make this more generic
        req.headers['x-now-route-matches']) {
            isSSG = true;
        } else if (!this.renderOpts.dev) {
            isSSG ||= !!prerenderManifest.routes[(0, _toroute.toRoute)(pathname)];
        }
        // Toggle whether or not this is a Data request
        const isNextDataRequest = !!(query.__nextDataReq || req.headers['x-nextjs-data'] && this.serverOptions.webServerConfig) && (isSSG || hasServerProps);
        /**
     * If true, this indicates that the request being made is for an app
     * prefetch request.
     */ const isPrefetchRSCRequest = (0, _requestmeta.getRequestMeta)(req, 'isPrefetchRSCRequest') ?? false;
        // NOTE: Don't delete headers[RSC] yet, it still needs to be used in renderToHTML later
        const isRSCRequest = (0, _requestmeta.getRequestMeta)(req, 'isRSCRequest') ?? false;
        // when we are handling a middleware prefetch and it doesn't
        // resolve to a static data route we bail early to avoid
        // unexpected SSR invocations
        if (!isSSG && req.headers['x-middleware-prefetch'] && !(is404Page || pathname === '/_error')) {
            res.setHeader(_constants2.MATCHED_PATH_HEADER, pathname);
            res.setHeader('x-middleware-skip', '1');
            res.setHeader('cache-control', 'private, no-cache, no-store, max-age=0, must-revalidate');
            res.body('{}').send();
            return null;
        }
        delete query.__nextDataReq;
        // normalize req.url for SSG paths as it is not exposed
        // to getStaticProps and the asPath should not expose /_next/data
        if (isSSG && this.minimalMode && req.headers[_constants2.MATCHED_PATH_HEADER] && req.url.startsWith('/_next/data')) {
            req.url = this.stripNextDataPath(req.url);
        }
        if (!!req.headers['x-nextjs-data'] && (!res.statusCode || res.statusCode === 200)) {
            res.setHeader('x-nextjs-matched-path', `${query.__nextLocale ? `/${query.__nextLocale}` : ''}${pathname}`);
        }
        let routeModule;
        if (components.routeModule) {
            routeModule = components.routeModule;
        }
        /**
     * If the route being rendered is an app page, and the ppr feature has been
     * enabled, then the given route _could_ support PPR.
     */ const couldSupportPPR = this.isAppPPREnabled && typeof routeModule !== 'undefined' && (0, _checks.isAppPageRouteModule)(routeModule);
        // When enabled, this will allow the use of the `?__nextppronly` query to
        // enable debugging of the static shell.
        const hasDebugStaticShellQuery = process.env.__NEXT_EXPERIMENTAL_STATIC_SHELL_DEBUGGING === '1' && typeof query.__nextppronly !== 'undefined' && couldSupportPPR;
        // When enabled, this will allow the use of the `?__nextppronly` query
        // to enable debugging of the fallback shell.
        const hasDebugFallbackShellQuery = hasDebugStaticShellQuery && query.__nextppronly === 'fallback';
        // This page supports PPR if it is marked as being `PARTIALLY_STATIC` in the
        // prerender manifest and this is an app page.
        const isRoutePPREnabled = couldSupportPPR && (((_this = prerenderManifest.routes[pathname] ?? prerenderManifest.dynamicRoutes[pathname]) == null ? void 0 : _this.renderingMode) === 'PARTIALLY_STATIC' || // Ideally we'd want to check the appConfig to see if this page has PPR
        // enabled or not, but that would require plumbing the appConfig through
        // to the server during development. We assume that the page supports it
        // but only during development.
        hasDebugStaticShellQuery && (this.renderOpts.dev === true || this.experimentalTestProxy === true));
        const isDebugStaticShell = hasDebugStaticShellQuery && isRoutePPREnabled;
        // We should enable debugging dynamic accesses when the static shell
        // debugging has been enabled and we're also in development mode.
        const isDebugDynamicAccesses = isDebugStaticShell && this.renderOpts.dev === true;
        const isDebugFallbackShell = hasDebugFallbackShellQuery && isRoutePPREnabled;
        // If we're in minimal mode, then try to get the postponed information from
        // the request metadata. If available, use it for resuming the postponed
        // render.
        const minimalPostponed = isRoutePPREnabled ? (0, _requestmeta.getRequestMeta)(req, 'postponed') : undefined;
        // If PPR is enabled, and this is a RSC request (but not a prefetch), then
        // we can use this fact to only generate the flight data for the request
        // because we can't cache the HTML (as it's also dynamic).
        const isDynamicRSCRequest = isRoutePPREnabled && isRSCRequest && !isPrefetchRSCRequest;
        // Need to read this before it's stripped by stripFlightHeaders. We don't
        // need to transfer it to the request meta because it's only read
        // within this function; the static segment data should have already been
        // generated, so we will always either return a static response or a 404.
        const segmentPrefetchHeader = req.headers[_approuterheaders.NEXT_ROUTER_SEGMENT_PREFETCH_HEADER.toLowerCase()];
        // we need to ensure the status code if /404 is visited directly
        if (is404Page && !isNextDataRequest && !isRSCRequest) {
            res.statusCode = 404;
        }
        // ensure correct status is set when visiting a status page
        // directly e.g. /500
        if (_constants.STATIC_STATUS_PAGES.includes(pathname)) {
            res.statusCode = parseInt(pathname.slice(1), 10);
        }
        if (// Server actions can use non-GET/HEAD methods.
        !isServerAction && // Resume can use non-GET/HEAD methods.
        !minimalPostponed && !is404Page && !is500Page && pathname !== '/_error' && req.method !== 'HEAD' && req.method !== 'GET' && (typeof components.Component === 'string' || isSSG)) {
            res.statusCode = 405;
            res.setHeader('Allow', [
                'GET',
                'HEAD'
            ]);
            await this.renderError(null, req, res, pathname);
            return null;
        }
        // handle static page
        if (typeof components.Component === 'string') {
            return {
                type: 'html',
                // TODO: Static pages should be serialized as RenderResult
                body: _renderresult.default.fromStatic(components.Component)
            };
        }
        // Ensure that if the `amp` query parameter is falsy that we remove it from
        // the query object. This ensures it won't be found by the `in` operator.
        if ('amp' in query && !query.amp) delete query.amp;
        if (opts.supportsDynamicResponse === true) {
            var _components_Document;
            const isBotRequest = (0, _isbot.isBot)(req.headers['user-agent'] || '');
            const isSupportedDocument = typeof ((_components_Document = components.Document) == null ? void 0 : _components_Document.getInitialProps) !== 'function' || // The built-in `Document` component also supports dynamic HTML for concurrent mode.
            _constants.NEXT_BUILTIN_DOCUMENT in components.Document;
            // Disable dynamic HTML in cases that we know it won't be generated,
            // so that we can continue generating a cache key when possible.
            // TODO-APP: should the first render for a dynamic app path
            // be static so we can collect revalidate and populate the
            // cache if there are no dynamic data requirements
            opts.supportsDynamicResponse = !isSSG && !isBotRequest && !query.amp && isSupportedDocument;
            opts.isBot = isBotRequest;
        }
        // In development, we always want to generate dynamic HTML.
        if (!isNextDataRequest && isAppPath && opts.dev) {
            opts.supportsDynamicResponse = true;
        }
        const defaultLocale = isSSG ? (_this_nextConfig_i18n = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n.defaultLocale : query.__nextDefaultLocale;
        const locale = query.__nextLocale;
        const locales = (_this_nextConfig_i18n1 = this.nextConfig.i18n) == null ? void 0 : _this_nextConfig_i18n1.locales;
        let previewData;
        let isPreviewMode = false;
        if (hasServerProps || isSSG || isAppPath) {
            // For the edge runtime, we don't support preview mode in SSG.
            if (process.env.NEXT_RUNTIME !== 'edge') {
                const { tryGetPreviewData } = require('./api-utils/node/try-get-preview-data');
                previewData = tryGetPreviewData(req, res, this.renderOpts.previewProps, !!this.nextConfig.experimental.multiZoneDraftMode);
                isPreviewMode = previewData !== false;
            }
        }
        // If this is a request for an app path that should be statically generated
        // and we aren't in the edge runtime, strip the flight headers so it will
        // generate the static response.
        if (isAppPath && !opts.dev && !isPreviewMode && isSSG && isRSCRequest && !isDynamicRSCRequest && (!(0, _isedgeruntime.isEdgeRuntime)(opts.runtime) || this.serverOptions.webServerConfig)) {
            (0, _stripflightheaders.stripFlightHeaders)(req.headers);
        }
        let isOnDemandRevalidate = false;
        let revalidateOnlyGenerated = false;
        if (isSSG) {
            ;
            ({ isOnDemandRevalidate, revalidateOnlyGenerated } = (0, _apiutils.checkIsOnDemandRevalidate)(req, this.renderOpts.previewProps));
        }
        if (isSSG && this.minimalMode && req.headers[_constants2.MATCHED_PATH_HEADER]) {
            // the url value is already correct when the matched-path header is set
            resolvedUrlPathname = urlPathname;
        }
        urlPathname = (0, _removetrailingslash.removeTrailingSlash)(urlPathname);
        resolvedUrlPathname = (0, _removetrailingslash.removeTrailingSlash)(resolvedUrlPathname);
        if (this.localeNormalizer) {
            resolvedUrlPathname = this.localeNormalizer.normalize(resolvedUrlPathname);
        }
        const handleRedirect = (pageData)=>{
            const redirect = {
                destination: pageData.pageProps.__N_REDIRECT,
                statusCode: pageData.pageProps.__N_REDIRECT_STATUS,
                basePath: pageData.pageProps.__N_REDIRECT_BASE_PATH
            };
            const statusCode = (0, _redirectstatus.getRedirectStatus)(redirect);
            const { basePath } = this.nextConfig;
            if (basePath && redirect.basePath !== false && redirect.destination.startsWith('/')) {
                redirect.destination = `${basePath}${redirect.destination}`;
            }
            if (redirect.destination.startsWith('/')) {
                redirect.destination = (0, _utils.normalizeRepeatedSlashes)(redirect.destination);
            }
            res.redirect(redirect.destination, statusCode).body(redirect.destination).send();
        };
        // remove /_next/data prefix from urlPathname so it matches
        // for direct page visit and /_next/data visit
        if (isNextDataRequest) {
            resolvedUrlPathname = this.stripNextDataPath(resolvedUrlPathname);
            urlPathname = this.stripNextDataPath(urlPathname);
        }
        let ssgCacheKey = null;
        if (!isPreviewMode && isSSG && !opts.supportsDynamicResponse && !isServerAction && !minimalPostponed && !isDynamicRSCRequest) {
            ssgCacheKey = `${locale ? `/${locale}` : ''}${(pathname === '/' || resolvedUrlPathname === '/') && locale ? '' : resolvedUrlPathname}${query.amp ? '.amp' : ''}`;
        }
        if ((is404Page || is500Page) && isSSG) {
            ssgCacheKey = `${locale ? `/${locale}` : ''}${pathname}${query.amp ? '.amp' : ''}`;
        }
        if (ssgCacheKey) {
            ssgCacheKey = (0, _decodepathparams.decodePathParams)(ssgCacheKey);
            // ensure /index and / is normalized to one key
            ssgCacheKey = ssgCacheKey === '/index' && pathname === '/' ? '/' : ssgCacheKey;
        }
        let protocol = 'https:';
        try {
            const parsedFullUrl = new URL((0, _requestmeta.getRequestMeta)(req, 'initURL') || '/', 'http://n');
            protocol = parsedFullUrl.protocol;
        } catch  {}
        // use existing incrementalCache instance if available
        const incrementalCache = globalThis.__incrementalCache || await this.getIncrementalCache({
            requestHeaders: Object.assign({}, req.headers),
            requestProtocol: protocol.substring(0, protocol.length - 1)
        });
        // TODO: investigate, this is not safe across multiple concurrent requests
        incrementalCache.resetRequestCache();
        const doRender = async ({ postponed, fallbackRouteParams })=>{
            // In development, we always want to generate dynamic HTML.
            let supportsDynamicResponse = // If we're in development, we always support dynamic HTML, unless it's
            // a data request, in which case we only produce static HTML.
            !isNextDataRequest && opts.dev === true || // If this is not SSG or does not have static paths, then it supports
            // dynamic HTML.
            !isSSG && !hasGetStaticPaths || // If this request has provided postponed data, it supports dynamic
            // HTML.
            typeof postponed === 'string' || // If this is a dynamic RSC request, then this render supports dynamic
            // HTML (it's dynamic).
            isDynamicRSCRequest;
            const origQuery = (0, _url.parse)(req.url || '', true).query;
            // clear any dynamic route params so they aren't in
            // the resolvedUrl
            if (opts.params) {
                Object.keys(opts.params).forEach((key)=>{
                    delete origQuery[key];
                });
            }
            const hadTrailingSlash = urlPathname !== '/' && this.nextConfig.trailingSlash;
            const resolvedUrl = (0, _url.format)({
                pathname: `${resolvedUrlPathname}${hadTrailingSlash ? '/' : ''}`,
                // make sure to only add query values from original URL
                query: origQuery
            });
            const renderOpts = {
                ...components,
                ...opts,
                ...isAppPath ? {
                    incrementalCache,
                    // This is a revalidation request if the request is for a static
                    // page and it is not being resumed from a postponed render and
                    // it is not a dynamic RSC request then it is a revalidation
                    // request.
                    isRevalidate: isSSG && !postponed && !isDynamicRSCRequest,
                    serverActions: this.nextConfig.experimental.serverActions
                } : {},
                isNextDataRequest,
                resolvedUrl,
                locale,
                locales,
                defaultLocale,
                multiZoneDraftMode: this.nextConfig.experimental.multiZoneDraftMode,
                // For getServerSideProps and getInitialProps we need to ensure we use the original URL
                // and not the resolved URL to prevent a hydration mismatch on
                // asPath
                resolvedAsPath: hasServerProps || hasGetInitialProps ? (0, _url.format)({
                    // we use the original URL pathname less the _next/data prefix if
                    // present
                    pathname: `${urlPathname}${hadTrailingSlash ? '/' : ''}`,
                    query: origQuery
                }) : resolvedUrl,
                experimental: {
                    ...opts.experimental,
                    isRoutePPREnabled
                },
                supportsDynamicResponse,
                isOnDemandRevalidate,
                isDraftMode: isPreviewMode,
                isServerAction,
                postponed,
                waitUntil: this.getWaitUntil(),
                onClose: res.onClose.bind(res),
                onAfterTaskError: undefined,
                // only available in dev
                setAppIsrStatus: this.setAppIsrStatus
            };
            if (isDebugStaticShell || isDebugDynamicAccesses) {
                supportsDynamicResponse = false;
                renderOpts.nextExport = true;
                renderOpts.supportsDynamicResponse = false;
                renderOpts.isStaticGeneration = true;
                renderOpts.isRevalidate = true;
                renderOpts.isDebugStaticShell = isDebugStaticShell;
                renderOpts.isDebugDynamicAccesses = isDebugDynamicAccesses;
            }
            // Legacy render methods will return a render result that needs to be
            // served by the server.
            let result;
            if (routeModule) {
                if ((0, _checks.isAppRouteRouteModule)(routeModule)) {
                    var _this_nextConfig_experimental;
                    if (// The type check here ensures that `req` is correctly typed, and the
                    // environment variable check provides dead code elimination.
                    process.env.NEXT_RUNTIME === 'edge' || !(0, _helpers.isNodeNextRequest)(req) || !(0, _helpers.isNodeNextResponse)(res)) {
                        throw new Error('Invariant: App Route Route Modules cannot be used in the edge runtime');
                    }
                    const context = {
                        params: opts.params,
                        prerenderManifest,
                        renderOpts: {
                            experimental: {
                                dynamicIO: renderOpts.experimental.dynamicIO,
                                authInterrupts: renderOpts.experimental.authInterrupts
                            },
                            supportsDynamicResponse,
                            incrementalCache,
                            cacheLifeProfiles: (_this_nextConfig_experimental = this.nextConfig.experimental) == null ? void 0 : _this_nextConfig_experimental.cacheLife,
                            isRevalidate: isSSG,
                            waitUntil: this.getWaitUntil(),
                            onClose: res.onClose.bind(res),
                            onAfterTaskError: undefined,
                            onInstrumentationRequestError: this.renderOpts.onInstrumentationRequestError,
                            buildId: this.renderOpts.buildId
                        }
                    };
                    try {
                        const request = _nextrequest.NextRequestAdapter.fromNodeNextRequest(req, (0, _nextrequest.signalFromNodeResponse)(res.originalResponse));
                        const response = await routeModule.handle(request, context);
                        req.fetchMetrics = context.renderOpts.fetchMetrics;
                        const cacheTags = context.renderOpts.collectedTags;
                        // If the request is for a static response, we can cache it so long
                        // as it's not edge.
                        if (isSSG) {
                            const blob = await response.blob();
                            // Copy the headers from the response.
                            const headers = (0, _utils3.toNodeOutgoingHttpHeaders)(response.headers);
                            if (cacheTags) {
                                headers[_constants2.NEXT_CACHE_TAGS_HEADER] = cacheTags;
                            }
                            if (!headers['content-type'] && blob.type) {
                                headers['content-type'] = blob.type;
                            }
                            const revalidate = typeof context.renderOpts.collectedRevalidate === 'undefined' || context.renderOpts.collectedRevalidate >= _constants2.INFINITE_CACHE ? false : context.renderOpts.collectedRevalidate;
                            // Create the cache entry for the response.
                            const cacheEntry = {
                                value: {
                                    kind: _responsecache.CachedRouteKind.APP_ROUTE,
                                    status: response.status,
                                    body: Buffer.from(await blob.arrayBuffer()),
                                    headers
                                },
                                revalidate,
                                isFallback: false
                            };
                            return cacheEntry;
                        }
                        // Send the response now that we have copied it into the cache.
                        await (0, _sendresponse.sendResponse)(req, res, response, context.renderOpts.pendingWaitUntil);
                        return null;
                    } catch (err) {
                        await this.instrumentationOnRequestError(err, req, {
                            routerKind: 'App Router',
                            routePath: pathname,
                            routeType: 'route',
                            revalidateReason: (0, _utils4.getRevalidateReason)(renderOpts)
                        });
                        // If this is during static generation, throw the error again.
                        if (isSSG) throw err;
                        _log.error(err);
                        // Otherwise, send a 500 response.
                        await (0, _sendresponse.sendResponse)(req, res, new Response(null, {
                            status: 500
                        }));
                        return null;
                    }
                } else if ((0, _checks.isPagesRouteModule)(routeModule) || (0, _checks.isAppPageRouteModule)(routeModule)) {
                    // An OPTIONS request to a page handler is invalid.
                    if (req.method === 'OPTIONS' && !is404Page) {
                        await (0, _sendresponse.sendResponse)(req, res, new Response(null, {
                            status: 400
                        }));
                        return null;
                    }
                    if ((0, _checks.isPagesRouteModule)(routeModule)) {
                        // Due to the way we pass data by mutating `renderOpts`, we can't extend
                        // the object here but only updating its `clientReferenceManifest` and
                        // `nextFontManifest` properties.
                        // https://github.com/vercel/next.js/blob/df7cbd904c3bd85f399d1ce90680c0ecf92d2752/packages/next/server/render.tsx#L947-L952
                        renderOpts.nextFontManifest = this.nextFontManifest;
                        renderOpts.clientReferenceManifest = components.clientReferenceManifest;
                        const request = (0, _helpers.isNodeNextRequest)(req) ? req.originalRequest : req;
                        const response = (0, _helpers.isNodeNextResponse)(res) ? res.originalResponse : res;
                        // Call the built-in render method on the module.
                        try {
                            result = await routeModule.render(// TODO: fix this type
                            // @ts-expect-error - preexisting accepted this
                            request, response, {
                                page: pathname,
                                params: opts.params,
                                query,
                                renderOpts
                            });
                        } catch (err) {
                            await this.instrumentationOnRequestError(err, req, {
                                routerKind: 'Pages Router',
                                routePath: pathname,
                                routeType: 'render',
                                revalidateReason: (0, _utils4.getRevalidateReason)({
                                    isRevalidate: isSSG,
                                    isOnDemandRevalidate: renderOpts.isOnDemandRevalidate
                                })
                            });
                            throw err;
                        }
                    } else {
                        const module1 = components.routeModule;
                        // Due to the way we pass data by mutating `renderOpts`, we can't extend the
                        // object here but only updating its `nextFontManifest` field.
                        // https://github.com/vercel/next.js/blob/df7cbd904c3bd85f399d1ce90680c0ecf92d2752/packages/next/server/render.tsx#L947-L952
                        renderOpts.nextFontManifest = this.nextFontManifest;
                        const context = {
                            page: is404Page ? '/404' : pathname,
                            params: opts.params,
                            query,
                            fallbackRouteParams,
                            renderOpts,
                            serverComponentsHmrCache: this.getServerComponentsHmrCache()
                        };
                        // TODO: adapt for putting the RDC inside the postponed data
                        // If we're in dev, and this isn't s prefetch or a server action,
                        // we should seed the resume data cache.
                        if (this.nextConfig.experimental.dynamicIO && this.renderOpts.dev && !isPrefetchRSCRequest && !isServerAction) {
                            const warmup = await module1.warmup(req, res, context);
                            // If the warmup is successful, we should use the resume data
                            // cache from the warmup.
                            if (warmup.metadata.devRenderResumeDataCache) {
                                renderOpts.devRenderResumeDataCache = warmup.metadata.devRenderResumeDataCache;
                            }
                        }
                        // Call the built-in render method on the module.
                        result = await module1.render(req, res, context);
                    }
                } else {
                    throw new Error('Invariant: Unknown route module type');
                }
            } else {
                // If we didn't match a page, we should fallback to using the legacy
                // render method.
                result = await this.renderHTML(req, res, pathname, query, renderOpts);
            }
            const { metadata } = result;
            const { headers = {}, // Add any fetch tags that were on the page to the response headers.
            fetchTags: cacheTags } = metadata;
            if (cacheTags) {
                headers[_constants2.NEXT_CACHE_TAGS_HEADER] = cacheTags;
            }
            // Pull any fetch metrics from the render onto the request.
            ;
            req.fetchMetrics = metadata.fetchMetrics;
            // we don't throw static to dynamic errors in dev as isSSG
            // is a best guess in dev since we don't have the prerender pass
            // to know whether the path is actually static or not
            if (isAppPath && isSSG && metadata.revalidate === 0 && !this.renderOpts.dev && !isRoutePPREnabled) {
                const staticBailoutInfo = metadata.staticBailoutInfo;
                const err = new Error(`Page changed from static to dynamic at runtime ${urlPathname}${(staticBailoutInfo == null ? void 0 : staticBailoutInfo.description) ? `, reason: ${staticBailoutInfo.description}` : ``}` + `\nsee more here https://nextjs.org/docs/messages/app-static-to-dynamic-error`);
                if (staticBailoutInfo == null ? void 0 : staticBailoutInfo.stack) {
                    const stack = staticBailoutInfo.stack;
                    err.stack = err.message + stack.substring(stack.indexOf('\n'));
                }
                throw err;
            }
            // Based on the metadata, we can determine what kind of cache result we
            // should return.
            // Handle `isNotFound`.
            if ('isNotFound' in metadata && metadata.isNotFound) {
                return {
                    value: null,
                    revalidate: metadata.revalidate,
                    isFallback: false
                };
            }
            // Handle `isRedirect`.
            if (metadata.isRedirect) {
                return {
                    value: {
                        kind: _responsecache.CachedRouteKind.REDIRECT,
                        props: metadata.pageData ?? metadata.flightData
                    },
                    revalidate: metadata.revalidate,
                    isFallback: false
                };
            }
            // Handle `isNull`.
            if (result.isNull) {
                return null;
            }
            // We now have a valid HTML result that we can return to the user.
            if (isAppPath) {
                return {
                    value: {
                        kind: _responsecache.CachedRouteKind.APP_PAGE,
                        html: result,
                        headers,
                        rscData: metadata.flightData,
                        postponed: metadata.postponed,
                        status: res.statusCode,
                        segmentData: metadata.segmentData
                    },
                    revalidate: metadata.revalidate,
                    isFallback: !!fallbackRouteParams
                };
            }
            return {
                value: {
                    kind: _responsecache.CachedRouteKind.PAGES,
                    html: result,
                    pageData: metadata.pageData ?? metadata.flightData,
                    headers,
                    status: isAppPath ? res.statusCode : undefined
                },
                revalidate: metadata.revalidate,
                isFallback: query.__nextFallback === 'true'
            };
        };
        let responseGenerator = async ({ hasResolved, previousCacheEntry, isRevalidating })=>{
            const isProduction = !this.renderOpts.dev;
            const didRespond = hasResolved || res.sent;
            // If we haven't found the static paths for the route, then do it now.
            if (!staticPaths && isDynamic) {
                if (hasGetStaticPaths) {
                    const pathsResult = await this.getStaticPaths({
                        pathname,
                        requestHeaders: req.headers,
                        isAppPath,
                        page: components.page
                    });
                    staticPaths = pathsResult.staticPaths;
                    fallbackMode = pathsResult.fallbackMode;
                } else {
                    staticPaths = undefined;
                    fallbackMode = _fallback.FallbackMode.NOT_FOUND;
                }
            }
            // When serving a bot request, we want to serve a blocking render and not
            // the prerendered page. This ensures that the correct content is served
            // to the bot in the head.
            if (fallbackMode === _fallback.FallbackMode.PRERENDER && (0, _isbot.isBot)(req.headers['user-agent'] || '')) {
                fallbackMode = _fallback.FallbackMode.BLOCKING_STATIC_RENDER;
            }
            // skip on-demand revalidate if cache is not present and
            // revalidate-if-generated is set
            if (isOnDemandRevalidate && revalidateOnlyGenerated && !previousCacheEntry && !this.minimalMode) {
                await this.render404(req, res);
                return null;
            }
            if ((previousCacheEntry == null ? void 0 : previousCacheEntry.isStale) === -1) {
                isOnDemandRevalidate = true;
            }
            // TODO: adapt for PPR
            // only allow on-demand revalidate for fallback: true/blocking
            // or for prerendered fallback: false paths
            if (isOnDemandRevalidate && (fallbackMode !== _fallback.FallbackMode.NOT_FOUND || previousCacheEntry)) {
                fallbackMode = _fallback.FallbackMode.BLOCKING_STATIC_RENDER;
            }
            // We use `ssgCacheKey` here as it is normalized to match the encoding
            // from getStaticPaths along with including the locale.
            //
            // We use the `resolvedUrlPathname` for the development case when this
            // is an app path since it doesn't include locale information.
            //
            // We decode the `resolvedUrlPathname` to correctly match the app path
            // with prerendered paths.
            let staticPathKey = ssgCacheKey;
            if (!staticPathKey && opts.dev && isAppPath) {
                staticPathKey = (0, _decodepathparams.decodePathParams)(resolvedUrlPathname);
            }
            if (staticPathKey && query.amp) {
                staticPathKey = staticPathKey.replace(/\.amp$/, '');
            }
            const isPageIncludedInStaticPaths = staticPathKey && (staticPaths == null ? void 0 : staticPaths.includes(staticPathKey));
            // When experimental compile is used, no pages have been prerendered,
            // so they should all be blocking.
            // @ts-expect-error internal field
            if (this.nextConfig.experimental.isExperimentalCompile) {
                fallbackMode = _fallback.FallbackMode.BLOCKING_STATIC_RENDER;
            }
            // When we did not respond from cache, we need to choose to block on
            // rendering or return a skeleton.
            //
            // - Data requests always block.
            // - Blocking mode fallback always blocks.
            // - Preview mode toggles all pages to be resolved in a blocking manner.
            // - Non-dynamic pages should block (though this is an impossible
            //   case in production).
            // - Dynamic pages should return their skeleton if not defined in
            //   getStaticPaths, then finish the data request on the client-side.
            //
            if (process.env.NEXT_RUNTIME !== 'edge' && !this.minimalMode && fallbackMode !== _fallback.FallbackMode.BLOCKING_STATIC_RENDER && staticPathKey && !didRespond && !isPreviewMode && isDynamic && (isProduction || !staticPaths || !isPageIncludedInStaticPaths)) {
                if (// In development, fall through to render to handle missing
                // getStaticPaths.
                (isProduction || staticPaths && (staticPaths == null ? void 0 : staticPaths.length) > 0) && // When fallback isn't present, abort this render so we 404
                fallbackMode === _fallback.FallbackMode.NOT_FOUND) {
                    throw new NoFallbackError();
                }
                let fallbackResponse;
                // If this is a pages router page.
                if ((0, _checks.isPagesRouteModule)(components.routeModule) && !isNextDataRequest) {
                    // We use the response cache here to handle the revalidation and
                    // management of the fallback shell.
                    fallbackResponse = await this.responseCache.get(isProduction ? locale ? `/${locale}${pathname}` : pathname : null, // This is the response generator for the fallback shell.
                    async ({ previousCacheEntry: previousFallbackCacheEntry = null })=>{
                        // For the pages router, fallbacks cannot be revalidated or
                        // generated in production. In the case of a missing fallback,
                        // we return null, but if it's being revalidated, we just return
                        // the previous fallback cache entry. This preserves the previous
                        // behavior.
                        if (isProduction) {
                            return (0, _utils5.toResponseCacheEntry)(previousFallbackCacheEntry);
                        }
                        // For the pages router, fallbacks can only be generated on
                        // demand in development, so if we're not in production, and we
                        // aren't a app path, then just add the __nextFallback query
                        // and render.
                        query.__nextFallback = 'true';
                        // We pass `undefined` and `null` as it doesn't apply to the pages
                        // router.
                        return doRender({
                            postponed: undefined,
                            fallbackRouteParams: null
                        });
                    }, {
                        routeKind: _routekind.RouteKind.PAGES,
                        incrementalCache,
                        isRoutePPREnabled,
                        isFallback: true
                    });
                } else if (isRoutePPREnabled && (0, _checks.isAppPageRouteModule)(components.routeModule) && !isRSCRequest) {
                    // We use the response cache here to handle the revalidation and
                    // management of the fallback shell.
                    fallbackResponse = await this.responseCache.get(isProduction ? pathname : null, // This is the response generator for the fallback shell.
                    async ()=>doRender({
                            // We pass `undefined` as rendering a fallback isn't resumed
                            // here.
                            postponed: undefined,
                            fallbackRouteParams: // If we're in production of we're debugging the fallback
                            // shell then we should postpone when dynamic params are
                            // accessed.
                            isProduction || isDebugFallbackShell ? (0, _fallbackparams.getFallbackRouteParams)(pathname) : null
                        }), {
                        routeKind: _routekind.RouteKind.APP_PAGE,
                        incrementalCache,
                        isRoutePPREnabled,
                        isFallback: true
                    });
                }
                // If the fallback response was set to null, then we should return null.
                if (fallbackResponse === null) return null;
                // Otherwise, if we did get a fallback response, we should return it.
                if (fallbackResponse) {
                    // Remove the revalidate from the response to prevent it from being
                    // used in the surrounding cache.
                    delete fallbackResponse.revalidate;
                    return fallbackResponse;
                }
            }
            // Only requests that aren't revalidating can be resumed. If we have the
            // minimal postponed data, then we should resume the render with it.
            const postponed = !isOnDemandRevalidate && !isRevalidating && minimalPostponed ? minimalPostponed : undefined;
            // When we're in minimal mode, if we're trying to debug the static shell,
            // we should just return nothing instead of resuming the dynamic render.
            if ((isDebugStaticShell || isDebugDynamicAccesses) && typeof postponed !== 'undefined') {
                return {
                    revalidate: 1,
                    isFallback: false,
                    value: {
                        kind: _responsecache.CachedRouteKind.PAGES,
                        html: _renderresult.default.fromStatic(''),
                        pageData: {},
                        headers: undefined,
                        status: undefined
                    }
                };
            }
            // If this is a dynamic route with PPR enabled and the default route
            // matches were set, then we should pass the fallback route params to
            // the renderer as this is a fallback revalidation request.
            const fallbackRouteParams = isDynamic && isRoutePPREnabled && ((0, _requestmeta.getRequestMeta)(req, 'didSetDefaultRouteMatches') || isDebugFallbackShell) ? (0, _fallbackparams.getFallbackRouteParams)(pathname) : null;
            // Perform the render.
            const result = await doRender({
                postponed,
                fallbackRouteParams
            });
            if (!result) return null;
            return {
                ...result,
                revalidate: result.revalidate
            };
        };
        const cacheEntry = await this.responseCache.get(ssgCacheKey, responseGenerator, {
            routeKind: // If the route module is not defined, we can assume it's a page being
            // rendered and thus check isAppPath.
            (routeModule == null ? void 0 : routeModule.definition.kind) ?? (isAppPath ? _routekind.RouteKind.APP_PAGE : _routekind.RouteKind.PAGES),
            incrementalCache,
            isOnDemandRevalidate,
            isPrefetch: req.headers.purpose === 'prefetch',
            isRoutePPREnabled
        });
        if (isPrefetchRSCRequest && typeof segmentPrefetchHeader === 'string') {
            var // This is always true at runtime but is needed to refine the type
            // of cacheEntry.value to CachedAppPageValue, because the outer
            // ResponseCacheEntry is not a discriminated union.
            _cacheEntry_value2;
            // This is a prefetch request issued by the client Segment Cache. These
            // should never reach the application layer (lambda). We should either
            // respond from the cache (HIT) or respond with 204 No Content (MISS).
            if (cacheEntry !== null && ((_cacheEntry_value2 = cacheEntry.value) == null ? void 0 : _cacheEntry_value2.kind) === _responsecache.CachedRouteKind.APP_PAGE && cacheEntry.value.segmentData) {
                const matchedSegment = cacheEntry.value.segmentData.get(segmentPrefetchHeader);
                if (matchedSegment !== undefined) {
                    // Cache hit
                    return {
                        type: 'rsc',
                        body: _renderresult.default.fromStatic(matchedSegment),
                        // TODO: Eventually this should use revalidate time of the
                        // individual segment, not the whole page.
                        revalidate: cacheEntry.revalidate
                    };
                }
            }
            // Cache miss. Either a cache entry for this route has not been generated,
            // or there's no match for the requested segment. Regardless, respond with
            // a 204 No Content. We don't bother to respond with 404 in cases where
            // the segment does not exist, because these requests are only issued by
            // the client cache.
            // TODO: If this is a request for the route tree (the special /_tree
            // segment), we should *always* respond with a tree, even if PPR
            // is disabled.
            res.statusCode = 204;
            if (isRoutePPREnabled) {
                // Set a header to indicate that PPR is enabled for this route. This
                // lets the client distinguish between a regular cache miss and a cache
                // miss due to PPR being disabled.
                // NOTE: Theoretically, when PPR is enabled, there should *never* be
                // a cache miss because we should generate a fallback route. So this
                // is mostly defensive.
                res.setHeader(_approuterheaders.NEXT_DID_POSTPONE_HEADER, '1');
            }
            return {
                type: 'rsc',
                body: _renderresult.default.fromStatic(''),
                revalidate: cacheEntry == null ? void 0 : cacheEntry.revalidate
            };
        }
        if (isPreviewMode) {
            res.setHeader('Cache-Control', 'private, no-cache, no-store, max-age=0, must-revalidate');
        }
        if (!cacheEntry) {
            if (ssgCacheKey && !(isOnDemandRevalidate && revalidateOnlyGenerated)) {
                // A cache entry might not be generated if a response is written
                // in `getInitialProps` or `getServerSideProps`, but those shouldn't
                // have a cache key. If we do have a cache key but we don't end up
                // with a cache entry, then either Next.js or the application has a
                // bug that needs fixing.
                throw new Error('invariant: cache entry required but not generated');
            }
            return null;
        }
        // If we're not in minimal mode and the cache entry that was returned was a
        // app page fallback, then we need to kick off the dynamic shell generation.
        if (ssgCacheKey && !this.minimalMode && isRoutePPREnabled && ((_cacheEntry_value = cacheEntry.value) == null ? void 0 : _cacheEntry_value.kind) === _responsecache.CachedRouteKind.APP_PAGE && cacheEntry.isFallback && !isOnDemandRevalidate && // When we're debugging the fallback shell, we don't want to regenerate
        // the route shell.
        !isDebugFallbackShell && process.env.DISABLE_ROUTE_SHELL_GENERATION !== 'true') {
            (0, _scheduler.scheduleOnNextTick)(async ()=>{
                try {
                    await this.responseCache.get(ssgCacheKey, ()=>doRender({
                            // We're an on-demand request, so we don't need to pass in the
                            // fallbackRouteParams.
                            fallbackRouteParams: null,
                            postponed: undefined
                        }), {
                        routeKind: _routekind.RouteKind.APP_PAGE,
                        incrementalCache,
                        isOnDemandRevalidate: true,
                        isPrefetch: false,
                        isRoutePPREnabled: true
                    });
                } catch (err) {
                    console.error('Error occurred while rendering dynamic shell', err);
                }
            });
        }
        const didPostpone = ((_cacheEntry_value1 = cacheEntry.value) == null ? void 0 : _cacheEntry_value1.kind) === _responsecache.CachedRouteKind.APP_PAGE && typeof cacheEntry.value.postponed === 'string';
        if (isSSG && // We don't want to send a cache header for requests that contain dynamic
        // data. If this is a Dynamic RSC request or wasn't a Prefetch RSC
        // request, then we should set the cache header.
        !isDynamicRSCRequest && (!didPostpone || isPrefetchRSCRequest)) {
            if (!this.minimalMode) {
                // set x-nextjs-cache header to match the header
                // we set for the image-optimizer
                res.setHeader('x-nextjs-cache', isOnDemandRevalidate ? 'REVALIDATED' : cacheEntry.isMiss ? 'MISS' : cacheEntry.isStale ? 'STALE' : 'HIT');
            }
            // Set a header used by the client router to signal the response is static
            // and should respect the `static` cache staleTime value.
            res.setHeader(_approuterheaders.NEXT_IS_PRERENDER_HEADER, '1');
        }
        const { value: cachedData } = cacheEntry;
        // If the cache value is an image, we should error early.
        if ((cachedData == null ? void 0 : cachedData.kind) === _responsecache.CachedRouteKind.IMAGE) {
            throw new Error('invariant SSG should not return an image cache value');
        }
        // Coerce the revalidate parameter from the render.
        let revalidate;
        // If this is a resume request in minimal mode it is streamed with dynamic
        // content and should not be cached.
        if (minimalPostponed) {
            revalidate = 0;
        } else if (this.minimalMode && isRSCRequest && !isPrefetchRSCRequest && isRoutePPREnabled) {
            revalidate = 0;
        } else if (!this.renderOpts.dev || hasServerProps && !isNextDataRequest) {
            // If this is a preview mode request, we shouldn't cache it
            if (isPreviewMode) {
                revalidate = 0;
            } else if (!isSSG) {
                if (!res.getHeader('Cache-Control')) {
                    revalidate = 0;
                }
            } else if (is404Page) {
                const notFoundRevalidate = (0, _requestmeta.getRequestMeta)(req, 'notFoundRevalidate');
                revalidate = typeof notFoundRevalidate === 'undefined' ? 0 : notFoundRevalidate;
            } else if (is500Page) {
                revalidate = 0;
            } else if (typeof cacheEntry.revalidate === 'number') {
                if (cacheEntry.revalidate < 1) {
                    throw new Error(`Invalid revalidate configuration provided: ${cacheEntry.revalidate} < 1`);
                }
                revalidate = cacheEntry.revalidate;
            } else if (cacheEntry.revalidate === false) {
                revalidate = _constants2.CACHE_ONE_YEAR;
            }
        }
        cacheEntry.revalidate = revalidate;
        // If there's a callback for `onCacheEntry`, call it with the cache entry
        // and the revalidate options.
        const onCacheEntry = (0, _requestmeta.getRequestMeta)(req, 'onCacheEntry');
        if (onCacheEntry) {
            var _cacheEntry_value3, _cacheEntry_value4;
            const finished = await onCacheEntry({
                ...cacheEntry,
                // TODO: remove this when upstream doesn't
                // always expect this value to be "PAGE"
                value: {
                    ...cacheEntry.value,
                    kind: ((_cacheEntry_value3 = cacheEntry.value) == null ? void 0 : _cacheEntry_value3.kind) === _responsecache.CachedRouteKind.APP_PAGE ? 'PAGE' : (_cacheEntry_value4 = cacheEntry.value) == null ? void 0 : _cacheEntry_value4.kind
                }
            }, {
                url: (0, _requestmeta.getRequestMeta)(req, 'initURL')
            });
            if (finished) {
                // TODO: maybe we have to end the request?
                return null;
            }
        }
        if (!cachedData) {
            // add revalidate metadata before rendering 404 page
            // so that we can use this as source of truth for the
            // cache-control header instead of what the 404 page returns
            // for the revalidate value
            (0, _requestmeta.addRequestMeta)(req, 'notFoundRevalidate', cacheEntry.revalidate);
            // If cache control is already set on the response we don't
            // override it to allow users to customize it via next.config
            if (typeof cacheEntry.revalidate !== 'undefined' && !res.getHeader('Cache-Control')) {
                res.setHeader('Cache-Control', (0, _revalidate.formatRevalidate)({
                    revalidate: cacheEntry.revalidate,
                    expireTime: this.nextConfig.expireTime
                }));
            }
            if (isNextDataRequest) {
                res.statusCode = 404;
                res.body('{"notFound":true}').send();
                return null;
            }
            if (this.renderOpts.dev) {
                query.__nextNotFoundSrcPage = pathname;
            }
            await this.render404(req, res, {
                pathname,
                query
            }, false);
            return null;
        } else if (cachedData.kind === _responsecache.CachedRouteKind.REDIRECT) {
            // If cache control is already set on the response we don't
            // override it to allow users to customize it via next.config
            if (typeof cacheEntry.revalidate !== 'undefined' && !res.getHeader('Cache-Control')) {
                res.setHeader('Cache-Control', (0, _revalidate.formatRevalidate)({
                    revalidate: cacheEntry.revalidate,
                    expireTime: this.nextConfig.expireTime
                }));
            }
            if (isNextDataRequest) {
                return {
                    type: 'json',
                    body: _renderresult.default.fromStatic(// @TODO: Handle flight data.
                    JSON.stringify(cachedData.props)),
                    revalidate: cacheEntry.revalidate
                };
            } else {
                await handleRedirect(cachedData.props);
                return null;
            }
        } else if (cachedData.kind === _responsecache.CachedRouteKind.APP_ROUTE) {
            const headers = (0, _utils3.fromNodeOutgoingHttpHeaders)(cachedData.headers);
            if (!(this.minimalMode && isSSG)) {
                headers.delete(_constants2.NEXT_CACHE_TAGS_HEADER);
            }
            // If cache control is already set on the response we don't
            // override it to allow users to customize it via next.config
            if (typeof cacheEntry.revalidate !== 'undefined' && !res.getHeader('Cache-Control') && !headers.get('Cache-Control')) {
                headers.set('Cache-Control', (0, _revalidate.formatRevalidate)({
                    revalidate: cacheEntry.revalidate,
                    expireTime: this.nextConfig.expireTime
                }));
            }
            await (0, _sendresponse.sendResponse)(req, res, new Response(cachedData.body, {
                headers,
                status: cachedData.status || 200
            }));
            return null;
        } else if (cachedData.kind === _responsecache.CachedRouteKind.APP_PAGE) {
            var _cachedData_headers;
            // If the request has a postponed state and it's a resume request we
            // should error.
            if (didPostpone && minimalPostponed) {
                throw new Error('Invariant: postponed state should not be present on a resume request');
            }
            if (cachedData.headers) {
                const headers = {
                    ...cachedData.headers
                };
                if (!this.minimalMode || !isSSG) {
                    delete headers[_constants2.NEXT_CACHE_TAGS_HEADER];
                }
                for (let [key, value] of Object.entries(headers)){
                    if (typeof value === 'undefined') continue;
                    if (Array.isArray(value)) {
                        for (const v of value){
                            res.appendHeader(key, v);
                        }
                    } else if (typeof value === 'number') {
                        value = value.toString();
                        res.appendHeader(key, value);
                    } else {
                        res.appendHeader(key, value);
                    }
                }
            }
            if (this.minimalMode && isSSG && ((_cachedData_headers = cachedData.headers) == null ? void 0 : _cachedData_headers[_constants2.NEXT_CACHE_TAGS_HEADER])) {
                res.setHeader(_constants2.NEXT_CACHE_TAGS_HEADER, cachedData.headers[_constants2.NEXT_CACHE_TAGS_HEADER]);
            }
            // If the request is a data request, then we shouldn't set the status code
            // from the response because it should always be 200. This should be gated
            // behind the experimental PPR flag.
            if (cachedData.status && (!isRSCRequest || !isRoutePPREnabled)) {
                res.statusCode = cachedData.status;
            }
            // Mark that the request did postpone.
            if (didPostpone) {
                res.setHeader(_approuterheaders.NEXT_DID_POSTPONE_HEADER, '1');
            }
            // we don't go through this block when preview mode is true
            // as preview mode is a dynamic request (bypasses cache) and doesn't
            // generate both HTML and payloads in the same request so continue to just
            // return the generated payload
            if (isRSCRequest && !isPreviewMode) {
                // If this is a dynamic RSC request, then stream the response.
                if (typeof cachedData.rscData === 'undefined') {
                    if (cachedData.postponed) {
                        throw new Error('Invariant: Expected postponed to be undefined');
                    }
                    return {
                        type: 'rsc',
                        body: cachedData.html,
                        // Dynamic RSC responses cannot be cached, even if they're
                        // configured with `force-static` because we have no way of
                        // distinguishing between `force-static` and pages that have no
                        // postponed state.
                        // TODO: distinguish `force-static` from pages with no postponed state (static)
                        revalidate: isDynamicRSCRequest ? 0 : cacheEntry.revalidate
                    };
                }
                // As this isn't a prefetch request, we should serve the static flight
                // data.
                return {
                    type: 'rsc',
                    body: _renderresult.default.fromStatic(cachedData.rscData),
                    revalidate: cacheEntry.revalidate
                };
            }
            // This is a request for HTML data.
            let body = cachedData.html;
            // If there's no postponed state, we should just serve the HTML. This
            // should also be the case for a resume request because it's completed
            // as a server render (rather than a static render).
            if (!didPostpone || this.minimalMode) {
                return {
                    type: 'html',
                    body,
                    revalidate: cacheEntry.revalidate
                };
            }
            // If we're debugging the static shell or the dynamic API accesses, we
            // should just serve the HTML without resuming the render. The returned
            // HTML will be the static shell so all the Dynamic API's will be used
            // during static generation.
            if (isDebugStaticShell || isDebugDynamicAccesses) {
                // Since we're not resuming the render, we need to at least add the
                // closing body and html tags to create valid HTML.
                body.chain(new ReadableStream({
                    start (controller) {
                        controller.enqueue(_encodedTags.ENCODED_TAGS.CLOSED.BODY_AND_HTML);
                        controller.close();
                    }
                }));
                return {
                    type: 'html',
                    body,
                    revalidate: 0
                };
            }
            // This request has postponed, so let's create a new transformer that the
            // dynamic data can pipe to that will attach the dynamic data to the end
            // of the response.
            const transformer = new TransformStream();
            body.chain(transformer.readable);
            // Perform the render again, but this time, provide the postponed state.
            // We don't await because we want the result to start streaming now, and
            // we've already chained the transformer's readable to the render result.
            doRender({
                postponed: cachedData.postponed,
                // This is a resume render, not a fallback render, so we don't need to
                // set this.
                fallbackRouteParams: null
            }).then(async (result)=>{
                var _result_value;
                if (!result) {
                    throw new Error('Invariant: expected a result to be returned');
                }
                if (((_result_value = result.value) == null ? void 0 : _result_value.kind) !== _responsecache.CachedRouteKind.APP_PAGE) {
                    var _result_value1;
                    throw new Error(`Invariant: expected a page response, got ${(_result_value1 = result.value) == null ? void 0 : _result_value1.kind}`);
                }
                // Pipe the resume result to the transformer.
                await result.value.html.pipeTo(transformer.writable);
            }).catch((err)=>{
                // An error occurred during piping or preparing the render, abort
                // the transformers writer so we can terminate the stream.
                transformer.writable.abort(err).catch((e)=>{
                    console.error("couldn't abort transformer", e);
                });
            });
            return {
                type: 'html',
                body,
                // We don't want to cache the response if it has postponed data because
                // the response being sent to the client it's dynamic parts are streamed
                // to the client on the same request.
                revalidate: 0
            };
        } else if (isNextDataRequest) {
            return {
                type: 'json',
                body: _renderresult.default.fromStatic(JSON.stringify(cachedData.pageData)),
                revalidate: cacheEntry.revalidate
            };
        } else {
            return {
                type: 'html',
                body: cachedData.html,
                revalidate: cacheEntry.revalidate
            };
        }
    }
    stripNextDataPath(path, stripLocale = true) {
        if (path.includes(this.buildId)) {
            const splitPath = path.substring(path.indexOf(this.buildId) + this.buildId.length);
            path = (0, _denormalizepagepath.denormalizePagePath)(splitPath.replace(/\.json$/, ''));
        }
        if (this.localeNormalizer && stripLocale) {
            return this.localeNormalizer.normalize(path);
        }
        return path;
    }
    // map the route to the actual bundle name
    getOriginalAppPaths(route) {
        if (this.enabledDirectories.app) {
            var _this_appPathRoutes;
            const originalAppPath = (_this_appPathRoutes = this.appPathRoutes) == null ? void 0 : _this_appPathRoutes[route];
            if (!originalAppPath) {
                return null;
            }
            return originalAppPath;
        }
        return null;
    }
    async renderPageComponent(ctx, bubbleNoFallback) {
        var _this_nextConfig_experimental_sri;
        const { query, pathname } = ctx;
        const appPaths = this.getOriginalAppPaths(pathname);
        const isAppPath = Array.isArray(appPaths);
        let page = pathname;
        if (isAppPath) {
            // the last item in the array is the root page, if there are parallel routes
            page = appPaths[appPaths.length - 1];
        }
        const result = await this.findPageComponents({
            page,
            query,
            params: ctx.renderOpts.params || {},
            isAppPath,
            sriEnabled: !!((_this_nextConfig_experimental_sri = this.nextConfig.experimental.sri) == null ? void 0 : _this_nextConfig_experimental_sri.algorithm),
            appPaths,
            // Ensuring for loading page component routes is done via the matcher.
            shouldEnsure: false
        });
        if (result) {
            (0, _tracer.getTracer)().setRootSpanAttribute('next.route', pathname);
            try {
                return await this.renderToResponseWithComponents(ctx, result);
            } catch (err) {
                const isNoFallbackError = err instanceof NoFallbackError;
                if (!isNoFallbackError || isNoFallbackError && bubbleNoFallback) {
                    throw err;
                }
            }
        }
        return false;
    }
    async renderToResponse(ctx) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.renderToResponse, {
            spanName: `rendering page`,
            attributes: {
                'next.route': ctx.pathname
            }
        }, async ()=>{
            return this.renderToResponseImpl(ctx);
        });
    }
    async renderToResponseImpl(ctx) {
        var _this_i18nProvider;
        const { res, query, pathname } = ctx;
        let page = pathname;
        const bubbleNoFallback = !!query._nextBubbleNoFallback;
        delete query[_approuterheaders.NEXT_RSC_UNION_QUERY];
        delete query._nextBubbleNoFallback;
        const options = {
            i18n: (_this_i18nProvider = this.i18nProvider) == null ? void 0 : _this_i18nProvider.fromQuery(pathname, query)
        };
        try {
            for await (const match of this.matchers.matchAll(pathname, options)){
                // when a specific invoke-output is meant to be matched
                // ensure a prior dynamic route/page doesn't take priority
                const invokeOutput = (0, _requestmeta.getRequestMeta)(ctx.req, 'invokeOutput');
                if (!this.minimalMode && typeof invokeOutput === 'string' && (0, _utils1.isDynamicRoute)(invokeOutput || '') && invokeOutput !== match.definition.pathname) {
                    continue;
                }
                const result = await this.renderPageComponent({
                    ...ctx,
                    pathname: match.definition.pathname,
                    renderOpts: {
                        ...ctx.renderOpts,
                        params: match.params
                    }
                }, bubbleNoFallback);
                if (result !== false) return result;
            }
            // currently edge functions aren't receiving the x-matched-path
            // header so we need to fallback to matching the current page
            // when we weren't able to match via dynamic route to handle
            // the rewrite case
            // @ts-expect-error extended in child class web-server
            if (this.serverOptions.webServerConfig) {
                // @ts-expect-error extended in child class web-server
                ctx.pathname = this.serverOptions.webServerConfig.page;
                const result = await this.renderPageComponent(ctx, bubbleNoFallback);
                if (result !== false) return result;
            }
        } catch (error) {
            const err = (0, _iserror.getProperError)(error);
            if (error instanceof _utils.MissingStaticPage) {
                console.error('Invariant: failed to load static page', JSON.stringify({
                    page,
                    url: ctx.req.url,
                    matchedPath: ctx.req.headers[_constants2.MATCHED_PATH_HEADER],
                    initUrl: (0, _requestmeta.getRequestMeta)(ctx.req, 'initURL'),
                    didRewrite: !!(0, _requestmeta.getRequestMeta)(ctx.req, 'rewroteURL'),
                    rewroteUrl: (0, _requestmeta.getRequestMeta)(ctx.req, 'rewroteURL')
                }, null, 2));
                throw err;
            }
            if (err instanceof NoFallbackError && bubbleNoFallback) {
                throw err;
            }
            if (err instanceof _utils.DecodeError || err instanceof _utils.NormalizeError) {
                res.statusCode = 400;
                return await this.renderErrorToResponse(ctx, err);
            }
            res.statusCode = 500;
            // if pages/500 is present we still need to trigger
            // /_error `getInitialProps` to allow reporting error
            if (await this.hasPage('/500')) {
                ctx.query.__nextCustomErrorRender = '1';
                await this.renderErrorToResponse(ctx, err);
                delete ctx.query.__nextCustomErrorRender;
            }
            const isWrappedError = err instanceof WrappedBuildError;
            if (!isWrappedError) {
                if (this.minimalMode && process.env.NEXT_RUNTIME !== 'edge' || this.renderOpts.dev) {
                    if ((0, _iserror.default)(err)) err.page = page;
                    throw err;
                }
                this.logError((0, _iserror.getProperError)(err));
            }
            const response = await this.renderErrorToResponse(ctx, isWrappedError ? err.innerError : err);
            return response;
        }
        if (this.getMiddleware() && !!ctx.req.headers['x-nextjs-data'] && (!res.statusCode || res.statusCode === 200 || res.statusCode === 404)) {
            res.setHeader('x-nextjs-matched-path', `${query.__nextLocale ? `/${query.__nextLocale}` : ''}${pathname}`);
            res.statusCode = 200;
            res.setHeader('content-type', 'application/json');
            res.body('{}');
            res.send();
            return null;
        }
        res.statusCode = 404;
        return this.renderErrorToResponse(ctx, null);
    }
    async renderToHTML(req, res, pathname, query = {}) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.renderToHTML, async ()=>{
            return this.renderToHTMLImpl(req, res, pathname, query);
        });
    }
    async renderToHTMLImpl(req, res, pathname, query = {}) {
        return this.getStaticHTML((ctx)=>this.renderToResponse(ctx), {
            req,
            res,
            pathname,
            query
        });
    }
    async renderError(err, req, res, pathname, query = {}, setHeaders = true) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.renderError, async ()=>{
            return this.renderErrorImpl(err, req, res, pathname, query, setHeaders);
        });
    }
    async renderErrorImpl(err, req, res, pathname, query = {}, setHeaders = true) {
        if (setHeaders) {
            res.setHeader('Cache-Control', 'private, no-cache, no-store, max-age=0, must-revalidate');
        }
        return this.pipe(async (ctx)=>{
            const response = await this.renderErrorToResponse(ctx, err);
            if (this.minimalMode && res.statusCode === 500) {
                throw err;
            }
            return response;
        }, {
            req,
            res,
            pathname,
            query
        });
    }
    async renderErrorToResponse(ctx, err) {
        return (0, _tracer.getTracer)().trace(_constants1.BaseServerSpan.renderErrorToResponse, async ()=>{
            return this.renderErrorToResponseImpl(ctx, err);
        });
    }
    async renderErrorToResponseImpl(ctx, err) {
        // Short-circuit favicon.ico in development to avoid compiling 404 page when the app has no favicon.ico.
        // Since favicon.ico is automatically requested by the browser.
        if (this.renderOpts.dev && ctx.pathname === '/favicon.ico') {
            return {
                type: 'html',
                body: _renderresult.default.fromStatic('')
            };
        }
        const { res, query } = ctx;
        try {
            let result = null;
            const is404 = res.statusCode === 404;
            let using404Page = false;
            if (is404) {
                if (this.enabledDirectories.app) {
                    // Use the not-found entry in app directory
                    result = await this.findPageComponents({
                        page: _constants.UNDERSCORE_NOT_FOUND_ROUTE_ENTRY,
                        query,
                        params: {},
                        isAppPath: true,
                        shouldEnsure: true,
                        url: ctx.req.url
                    });
                    using404Page = result !== null;
                }
                if (!result && await this.hasPage('/404')) {
                    result = await this.findPageComponents({
                        page: '/404',
                        query,
                        params: {},
                        isAppPath: false,
                        // Ensuring can't be done here because you never "match" a 404 route.
                        shouldEnsure: true,
                        url: ctx.req.url
                    });
                    using404Page = result !== null;
                }
            }
            let statusPage = `/${res.statusCode}`;
            if (!ctx.query.__nextCustomErrorRender && !result && _constants.STATIC_STATUS_PAGES.includes(statusPage)) {
                // skip ensuring /500 in dev mode as it isn't used and the
                // dev overlay is used instead
                if (statusPage !== '/500' || !this.renderOpts.dev) {
                    result = await this.findPageComponents({
                        page: statusPage,
                        query,
                        params: {},
                        isAppPath: false,
                        // Ensuring can't be done here because you never "match" a 500
                        // route.
                        shouldEnsure: true,
                        url: ctx.req.url
                    });
                }
            }
            if (!result) {
                result = await this.findPageComponents({
                    page: '/_error',
                    query,
                    params: {},
                    isAppPath: false,
                    // Ensuring can't be done here because you never "match" an error
                    // route.
                    shouldEnsure: true,
                    url: ctx.req.url
                });
                statusPage = '/_error';
            }
            if (process.env.NODE_ENV !== 'production' && !using404Page && await this.hasPage('/_error') && !await this.hasPage('/404')) {
                this.customErrorNo404Warn();
            }
            if (!result) {
                // this can occur when a project directory has been moved/deleted
                // which is handled in the parent process in development
                if (this.renderOpts.dev) {
                    return {
                        type: 'html',
                        // wait for dev-server to restart before refreshing
                        body: _renderresult.default.fromStatic(`
              <pre>missing required error components, refreshing...</pre>
              <script>
                async function check() {
                  const res = await fetch(location.href).catch(() => ({}))

                  if (res.status === 200) {
                    location.reload()
                  } else {
                    setTimeout(check, 1000)
                  }
                }
                check()
              </script>`)
                    };
                }
                throw new WrappedBuildError(new Error('missing required error components'));
            }
            // If the page has a route module, use it for the new match. If it doesn't
            // have a route module, remove the match.
            if (result.components.routeModule) {
                (0, _requestmeta.addRequestMeta)(ctx.req, 'match', {
                    definition: result.components.routeModule.definition,
                    params: undefined
                });
            } else {
                (0, _requestmeta.removeRequestMeta)(ctx.req, 'match');
            }
            try {
                return await this.renderToResponseWithComponents({
                    ...ctx,
                    pathname: statusPage,
                    renderOpts: {
                        ...ctx.renderOpts,
                        err
                    }
                }, result);
            } catch (maybeFallbackError) {
                if (maybeFallbackError instanceof NoFallbackError) {
                    throw new Error('invariant: failed to render error page');
                }
                throw maybeFallbackError;
            }
        } catch (error) {
            const renderToHtmlError = (0, _iserror.getProperError)(error);
            const isWrappedError = renderToHtmlError instanceof WrappedBuildError;
            if (!isWrappedError) {
                this.logError(renderToHtmlError);
            }
            res.statusCode = 500;
            const fallbackComponents = await this.getFallbackErrorComponents(ctx.req.url);
            if (fallbackComponents) {
                // There was an error, so use it's definition from the route module
                // to add the match to the request.
                (0, _requestmeta.addRequestMeta)(ctx.req, 'match', {
                    definition: fallbackComponents.routeModule.definition,
                    params: undefined
                });
                return this.renderToResponseWithComponents({
                    ...ctx,
                    pathname: '/_error',
                    renderOpts: {
                        ...ctx.renderOpts,
                        // We render `renderToHtmlError` here because `err` is
                        // already captured in the stacktrace.
                        err: isWrappedError ? renderToHtmlError.innerError : renderToHtmlError
                    }
                }, {
                    query,
                    components: fallbackComponents
                });
            }
            return {
                type: 'html',
                body: _renderresult.default.fromStatic('Internal Server Error')
            };
        }
    }
    async renderErrorToHTML(err, req, res, pathname, query = {}) {
        return this.getStaticHTML((ctx)=>this.renderErrorToResponse(ctx, err), {
            req,
            res,
            pathname,
            query
        });
    }
    async render404(req, res, parsedUrl, setHeaders = true) {
        const { pathname, query } = parsedUrl ? parsedUrl : (0, _url.parse)(req.url, true);
        if (this.nextConfig.i18n) {
            query.__nextLocale ||= this.nextConfig.i18n.defaultLocale;
            query.__nextDefaultLocale ||= this.nextConfig.i18n.defaultLocale;
        }
        res.statusCode = 404;
        return this.renderError(null, req, res, pathname, query, setHeaders);
    }
}

//# sourceMappingURL=base-server.js.map