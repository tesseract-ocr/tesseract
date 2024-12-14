"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "exportPages", {
    enumerable: true,
    get: function() {
        return exportPages;
    }
});
require("../server/node-environment");
const _path = require("path");
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _loadcomponents = require("../server/load-components");
const _isdynamic = require("../shared/lib/router/utils/is-dynamic");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _normalizelocalepath = require("../shared/lib/i18n/normalize-locale-path");
const _trace = require("../trace");
const _setuphttpagentenv = require("../server/setup-http-agent-env");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
const _requestmeta = require("../server/request-meta");
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _mockrequest = require("../server/lib/mock-request");
const _isapprouteroute = require("../lib/is-app-route-route");
const _ciinfo = require("../server/ci-info");
const _approute = require("./routes/app-route");
const _apppage = require("./routes/app-page");
const _pages = require("./routes/pages");
const _getparams = require("./helpers/get-params");
const _createincrementalcache = require("./helpers/create-incremental-cache");
const _ispostpone = require("../server/lib/router-utils/is-postpone");
const _isdynamicusageerror = require("./helpers/is-dynamic-usage-error");
const _bailouttocsr = require("../shared/lib/lazy-dynamic/bailout-to-csr");
const _turborepoaccesstrace = require("../build/turborepo-access-trace");
const _fallbackparams = require("../server/request/fallback-params");
const _needsexperimentalreact = require("../lib/needs-experimental-react");
const _staticgenerationbailout = require("../client/components/static-generation-bailout");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
process.env.NEXT_IS_EXPORT_WORKER = 'true';
const envConfig = require('../shared/lib/runtime-config.external');
globalThis.__NEXT_DATA__ = {
    nextExport: true
};
class TimeoutError extends Error {
    constructor(...args){
        super(...args), this.code = 'NEXT_EXPORT_TIMEOUT_ERROR';
    }
}
class ExportPageError extends Error {
    constructor(...args){
        super(...args), this.code = 'NEXT_EXPORT_PAGE_ERROR';
    }
}
async function exportPageImpl(input, fileWriter) {
    var _req_url;
    const { path, pathMap, distDir, pagesDataDir, buildExport = false, serverRuntimeConfig, subFolders = false, optimizeCss, disableOptimizedLoading, debugOutput = false, enableExperimentalReact, ampValidatorPath, trailingSlash } = input;
    if (enableExperimentalReact) {
        process.env.__NEXT_EXPERIMENTAL_REACT = 'true';
    }
    const { page, // The parameters that are currently unknown.
    _fallbackRouteParams = [], // Check if this is an `app/` page.
    _isAppDir: isAppDir = false, // Check if this should error when dynamic usage is detected.
    _isDynamicError: isDynamicError = false, // If this page supports partial prerendering, then we need to pass that to
    // the renderOpts.
    _isRoutePPREnabled: isRoutePPREnabled, // If this is a prospective render, we don't actually want to persist the
    // result, we just want to use it to error the build if there's a problem.
    _isProspectiveRender: isProspectiveRender = false, // Pull the original query out.
    query: originalQuery = {} } = pathMap;
    const fallbackRouteParams = (0, _fallbackparams.getFallbackRouteParams)(_fallbackRouteParams);
    let query = {
        ...originalQuery
    };
    const pathname = (0, _apppaths.normalizeAppPath)(page);
    const isDynamic = (0, _isdynamic.isDynamicRoute)(page);
    const outDir = isAppDir ? (0, _path.join)(distDir, 'server/app') : input.outDir;
    const filePath = (0, _normalizepagepath.normalizePagePath)(path);
    const ampPath = `${filePath}.amp`;
    let renderAmpPath = ampPath;
    let updatedPath = query.__nextSsgPath || path;
    delete query.__nextSsgPath;
    let locale = query.__nextLocale || input.renderOpts.locale;
    delete query.__nextLocale;
    if (input.renderOpts.locale) {
        const localePathResult = (0, _normalizelocalepath.normalizeLocalePath)(path, input.renderOpts.locales);
        if (localePathResult.detectedLocale) {
            updatedPath = localePathResult.pathname;
            locale = localePathResult.detectedLocale;
            if (locale === input.renderOpts.defaultLocale) {
                renderAmpPath = `${(0, _normalizepagepath.normalizePagePath)(updatedPath)}.amp`;
            }
        }
    }
    // We need to show a warning if they try to provide query values
    // for an auto-exported page since they won't be available
    const hasOrigQueryValues = Object.keys(originalQuery).length > 0;
    // Check if the page is a specified dynamic route
    const { pathname: nonLocalizedPath } = (0, _normalizelocalepath.normalizeLocalePath)(path, input.renderOpts.locales);
    let params;
    if (isDynamic && page !== nonLocalizedPath) {
        const normalizedPage = isAppDir ? (0, _apppaths.normalizeAppPath)(page) : page;
        params = (0, _getparams.getParams)(normalizedPage, updatedPath);
    }
    const { req, res } = (0, _mockrequest.createRequestResponseMocks)({
        url: updatedPath
    });
    // If this is a status code page, then set the response code.
    for (const statusCode of [
        404,
        500
    ]){
        if ([
            `/${statusCode}`,
            `/${statusCode}.html`,
            `/${statusCode}/index.html`
        ].some((p)=>p === updatedPath || `/${locale}${p}` === updatedPath)) {
            res.statusCode = statusCode;
        }
    }
    // Ensure that the URL has a trailing slash if it's configured.
    if (trailingSlash && !((_req_url = req.url) == null ? void 0 : _req_url.endsWith('/'))) {
        req.url += '/';
    }
    if (locale && buildExport && input.renderOpts.domainLocales && input.renderOpts.domainLocales.some((dl)=>{
        var _dl_locales;
        return dl.defaultLocale === locale || ((_dl_locales = dl.locales) == null ? void 0 : _dl_locales.includes(locale || ''));
    })) {
        (0, _requestmeta.addRequestMeta)(req, 'isLocaleDomain', true);
    }
    envConfig.setConfig({
        serverRuntimeConfig,
        publicRuntimeConfig: input.renderOpts.runtimeConfig
    });
    const getHtmlFilename = (p)=>subFolders ? `${p}${_path.sep}index.html` : `${p}.html`;
    let htmlFilename = getHtmlFilename(filePath);
    // dynamic routes can provide invalid extensions e.g. /blog/[...slug] returns an
    // extension of `.slug]`
    const pageExt = isDynamic || isAppDir ? '' : (0, _path.extname)(page);
    const pathExt = isDynamic || isAppDir ? '' : (0, _path.extname)(path);
    // force output 404.html for backwards compat
    if (path === '/404.html') {
        htmlFilename = path;
    } else if (pageExt !== pathExt && pathExt !== '') {
        const isBuiltinPaths = [
            '/500',
            '/404'
        ].some((p)=>p === path || p === path + '.html');
        // If the ssg path has .html extension, and it's not builtin paths, use it directly
        // Otherwise, use that as the filename instead
        const isHtmlExtPath = !isBuiltinPaths && path.endsWith('.html');
        htmlFilename = isHtmlExtPath ? getHtmlFilename(path) : path;
    } else if (path === '/') {
        // If the path is the root, just use index.html
        htmlFilename = 'index.html';
    }
    const baseDir = (0, _path.join)(outDir, (0, _path.dirname)(htmlFilename));
    let htmlFilepath = (0, _path.join)(outDir, htmlFilename);
    await _promises.default.mkdir(baseDir, {
        recursive: true
    });
    const components = await (0, _loadcomponents.loadComponents)({
        distDir,
        page,
        isAppPath: isAppDir
    });
    // Handle App Routes.
    if (isAppDir && (0, _isapprouteroute.isAppRouteRoute)(page)) {
        return (0, _approute.exportAppRoute)(req, res, params, page, components.routeModule, input.renderOpts.incrementalCache, input.renderOpts.cacheLifeProfiles, htmlFilepath, fileWriter, input.renderOpts.experimental, input.renderOpts.buildId);
    }
    const renderOpts = {
        ...components,
        ...input.renderOpts,
        ampPath: renderAmpPath,
        params,
        optimizeCss,
        disableOptimizedLoading,
        locale,
        supportsDynamicResponse: false,
        experimental: {
            ...input.renderOpts.experimental,
            isRoutePPREnabled
        }
    };
    if (_ciinfo.hasNextSupport) {
        renderOpts.isRevalidate = true;
    }
    // Handle App Pages
    if (isAppDir) {
        // If this is a prospective render, don't return any metrics or revalidate
        // timings as we aren't persisting this render (it was only to error).
        if (isProspectiveRender) {
            return (0, _apppage.prospectiveRenderAppPage)(req, res, page, pathname, query, fallbackRouteParams, renderOpts);
        }
        return (0, _apppage.exportAppPage)(req, res, page, path, pathname, query, fallbackRouteParams, renderOpts, htmlFilepath, debugOutput, isDynamicError, fileWriter);
    }
    return (0, _pages.exportPagesPage)(req, res, path, page, query, params, htmlFilepath, htmlFilename, ampPath, subFolders, outDir, ampValidatorPath, pagesDataDir, buildExport, isDynamic, hasOrigQueryValues, renderOpts, components, fileWriter);
}
async function exportPages(input) {
    const { exportPathMap, paths, dir, distDir, outDir, cacheHandler, cacheMaxMemorySize, fetchCacheKeyPrefix, pagesDataDir, renderOpts, nextConfig, options } = input;
    // If the fetch cache was enabled, we need to create an incremental
    // cache instance for this page.
    const incrementalCache = await (0, _createincrementalcache.createIncrementalCache)({
        cacheHandler,
        cacheMaxMemorySize,
        fetchCacheKeyPrefix,
        distDir,
        dir,
        dynamicIO: Boolean(nextConfig.experimental.dynamicIO),
        // skip writing to disk in minimal mode for now, pending some
        // changes to better support it
        flushToDisk: !_ciinfo.hasNextSupport,
        cacheHandlers: nextConfig.experimental.cacheHandlers
    });
    renderOpts.incrementalCache = incrementalCache;
    const maxConcurrency = nextConfig.experimental.staticGenerationMaxConcurrency ?? 8;
    const results = [];
    const exportPageWithRetry = async (path, maxAttempts)=>{
        const pathMap = exportPathMap[path];
        const { page } = exportPathMap[path];
        const pageKey = page !== path ? `${page}: ${path}` : path;
        let attempt = 0;
        let result;
        while(attempt < maxAttempts){
            try {
                var _nextConfig_experimental_amp;
                result = await Promise.race([
                    exportPage({
                        path,
                        pathMap,
                        distDir,
                        outDir,
                        pagesDataDir,
                        renderOpts,
                        ampValidatorPath: ((_nextConfig_experimental_amp = nextConfig.experimental.amp) == null ? void 0 : _nextConfig_experimental_amp.validator) || undefined,
                        trailingSlash: nextConfig.trailingSlash,
                        serverRuntimeConfig: nextConfig.serverRuntimeConfig,
                        subFolders: nextConfig.trailingSlash && !options.buildExport,
                        buildExport: options.buildExport,
                        optimizeCss: nextConfig.experimental.optimizeCss,
                        disableOptimizedLoading: nextConfig.experimental.disableOptimizedLoading,
                        parentSpanId: input.parentSpanId,
                        httpAgentOptions: nextConfig.httpAgentOptions,
                        debugOutput: options.debugOutput,
                        enableExperimentalReact: (0, _needsexperimentalreact.needsExperimentalReact)(nextConfig)
                    }),
                    // If exporting the page takes longer than the timeout, reject the promise.
                    new Promise((_, reject)=>{
                        setTimeout(()=>{
                            reject(new TimeoutError());
                        }, nextConfig.staticPageGenerationTimeout * 1000);
                    })
                ]);
                // If there was an error in the export, throw it immediately. In the catch block, we might retry the export,
                // or immediately fail the build, depending on user configuration. We might also continue on and attempt other pages.
                if (result && 'error' in result) {
                    throw new ExportPageError();
                }
                break;
            } catch (err) {
                // The only error that should be caught here is an ExportError, as `exportPage` doesn't throw and instead returns an object with an `error` property.
                // This is an overly cautious check to ensure that we don't accidentally catch an unexpected error.
                if (!(err instanceof ExportPageError || err instanceof TimeoutError)) {
                    throw err;
                }
                if (err instanceof TimeoutError) {
                    // If the export times out, we will restart the worker up to 3 times.
                    maxAttempts = 3;
                }
                // We've reached the maximum number of attempts
                if (attempt >= maxAttempts - 1) {
                    // Log a message if we've reached the maximum number of attempts.
                    // We only care to do this if maxAttempts was configured.
                    if (maxAttempts > 1) {
                        console.info(`Failed to build ${pageKey} after ${maxAttempts} attempts.`);
                    }
                    // If prerenderEarlyExit is enabled, we'll exit the build immediately.
                    if (nextConfig.experimental.prerenderEarlyExit) {
                        console.error(`Export encountered an error on ${pageKey}, exiting the build.`);
                        process.exit(1);
                    } else {
                    // Otherwise, this is a no-op. The build will continue, and a summary of failed pages will be displayed at the end.
                    }
                } else {
                    // Otherwise, we have more attempts to make. Wait before retrying
                    if (err instanceof TimeoutError) {
                        console.info(`Failed to build ${pageKey} (attempt ${attempt + 1} of ${maxAttempts}) because it took more than ${nextConfig.staticPageGenerationTimeout} seconds. Retrying again shortly.`);
                    } else {
                        console.info(`Failed to build ${pageKey} (attempt ${attempt + 1} of ${maxAttempts}). Retrying again shortly.`);
                    }
                    await new Promise((r)=>setTimeout(r, Math.random() * 500));
                }
            }
            attempt++;
        }
        return {
            result,
            path,
            pageKey
        };
    };
    for(let i = 0; i < paths.length; i += maxConcurrency){
        const subset = paths.slice(i, i + maxConcurrency);
        const subsetResults = await Promise.all(subset.map((path)=>exportPageWithRetry(path, nextConfig.experimental.staticGenerationRetryCount ?? 1)));
        results.push(...subsetResults);
    }
    return results;
}
async function exportPage(input) {
    (0, _trace.trace)('export-page', input.parentSpanId).setAttribute('path', input.path);
    // Configure the http agent.
    (0, _setuphttpagentenv.setHttpClientAndAgentOptions)({
        httpAgentOptions: input.httpAgentOptions
    });
    const files = [];
    const baseFileWriter = async (type, path, content, encodingOptions = 'utf-8')=>{
        await _promises.default.mkdir((0, _path.dirname)(path), {
            recursive: true
        });
        await _promises.default.writeFile(path, content, encodingOptions);
        files.push({
            type,
            path
        });
    };
    const exportPageSpan = (0, _trace.trace)('export-page-worker', input.parentSpanId);
    const start = Date.now();
    const turborepoAccessTraceResult = new _turborepoaccesstrace.TurborepoAccessTraceResult();
    // Export the page.
    let result;
    try {
        result = await exportPageSpan.traceAsyncFn(()=>(0, _turborepoaccesstrace.turborepoTraceAccess)(()=>exportPageImpl(input, baseFileWriter), turborepoAccessTraceResult));
        // If there was no result, then we can exit early.
        if (!result) return;
        // If there was an error, then we can exit early.
        if ('error' in result) {
            return {
                error: result.error,
                duration: Date.now() - start,
                files: []
            };
        }
    } catch (err) {
        console.error(`Error occurred prerendering page "${input.path}". Read more: https://nextjs.org/docs/messages/prerender-error`);
        // bailoutToCSRError errors should not leak to the user as they are not actionable; they're
        // a framework signal
        if (!(0, _bailouttocsr.isBailoutToCSRError)(err)) {
            // A static generation bailout error is a framework signal to fail static generation but
            // and will encode a reason in the error message. If there is a message, we'll print it.
            // Otherwise there's nothing to show as we don't want to leak an error internal error stack to the user.
            if ((0, _staticgenerationbailout.isStaticGenBailoutError)(err)) {
                if (err.message) {
                    console.error(`Error: ${err.message}`);
                }
            } else if ((0, _iserror.default)(err) && err.stack) {
                console.error(err.stack);
            } else {
                console.error(err);
            }
        }
        return {
            error: true,
            duration: Date.now() - start,
            files: []
        };
    }
    // Notify the parent process that we processed a page (used by the progress activity indicator)
    process.send == null ? void 0 : process.send.call(process, [
        3,
        {
            type: 'activity'
        }
    ]);
    // Otherwise we can return the result.
    return {
        duration: Date.now() - start,
        files,
        ampValidations: result.ampValidations,
        revalidate: result.revalidate,
        metadata: result.metadata,
        ssgNotFound: result.ssgNotFound,
        hasEmptyPrelude: result.hasEmptyPrelude,
        hasPostponed: result.hasPostponed,
        turborepoAccessTraceResult: turborepoAccessTraceResult.serialize(),
        fetchMetrics: result.fetchMetrics
    };
}
process.on('unhandledRejection', (err)=>{
    // if it's a postpone error, it'll be handled later
    // when the postponed promise is actually awaited.
    if ((0, _ispostpone.isPostpone)(err)) {
        return;
    }
    // we don't want to log these errors
    if ((0, _isdynamicusageerror.isDynamicUsageError)(err)) {
        return;
    }
    console.error(err);
});
process.on('rejectionHandled', ()=>{
// It is ok to await a Promise late in Next.js as it allows for better
// prefetching patterns to avoid waterfalls. We ignore logging these.
// We should've already errored in anyway unhandledRejection.
});
const FATAL_UNHANDLED_NEXT_API_EXIT_CODE = 78;
process.on('uncaughtException', (err)=>{
    if ((0, _isdynamicusageerror.isDynamicUsageError)(err)) {
        console.error('A Next.js API that uses exceptions to signal framework behavior was uncaught. This suggests improper usage of a Next.js API. The original error is printed below and the build will now exit.');
        console.error(err);
        process.exit(FATAL_UNHANDLED_NEXT_API_EXIT_CODE);
    } else {
        console.error(err);
    }
});

//# sourceMappingURL=worker.js.map