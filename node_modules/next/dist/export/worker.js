"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return exportPage;
    }
});
require("../server/node-environment");
const _path = require("path");
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _loadcomponents = require("../server/load-components");
const _isdynamic = require("../shared/lib/router/utils/is-dynamic");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _require = require("../server/require");
const _normalizelocalepath = require("../shared/lib/i18n/normalize-locale-path");
const _trace = require("../trace");
const _setuphttpagentenv = require("../server/setup-http-agent-env");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
const _requestmeta = require("../server/request-meta");
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _mockrequest = require("../server/lib/mock-request");
const _isapprouteroute = require("../lib/is-app-route-route");
const _ciinfo = require("../telemetry/ci-info");
const _approute = require("./routes/app-route");
const _apppage = require("./routes/app-page");
const _pages = require("./routes/pages");
const _getparams = require("./helpers/get-params");
const _createincrementalcache = require("./helpers/create-incremental-cache");
const _ispostpone = require("../server/lib/router-utils/is-postpone");
const _isdynamicusageerror = require("./helpers/is-dynamic-usage-error");
const _bailouttocsr = require("../shared/lib/lazy-dynamic/bailout-to-csr");
const _turborepoaccesstrace = require("../build/turborepo-access-trace");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
process.env.NEXT_IS_EXPORT_WORKER = "true";
const envConfig = require("../shared/lib/runtime-config.external");
globalThis.__NEXT_DATA__ = {
    nextExport: true
};
async function exportPageImpl(input, fileWriter) {
    const { dir, path, pathMap, distDir, pagesDataDir, buildExport = false, serverRuntimeConfig, subFolders = false, optimizeFonts, optimizeCss, disableOptimizedLoading, debugOutput = false, cacheMaxMemorySize, fetchCache, fetchCacheKeyPrefix, cacheHandler, enableExperimentalReact, ampValidatorPath, trailingSlash, enabledDirectories } = input;
    if (enableExperimentalReact) {
        process.env.__NEXT_EXPERIMENTAL_REACT = "true";
    }
    const { page, // Check if this is an `app/` page.
    _isAppDir: isAppDir = false, // TODO: use this when we've re-enabled app prefetching https://github.com/vercel/next.js/pull/58609
    // // Check if this is an `app/` prefix request.
    // _isAppPrefetch: isAppPrefetch = false,
    // Check if this should error when dynamic usage is detected.
    _isDynamicError: isDynamicError = false, // Pull the original query out.
    query: originalQuery = {} } = pathMap;
    try {
        var _req_url;
        let query = {
            ...originalQuery
        };
        const pathname = (0, _apppaths.normalizeAppPath)(page);
        const isDynamic = (0, _isdynamic.isDynamicRoute)(page);
        const outDir = isAppDir ? (0, _path.join)(distDir, "server/app") : input.outDir;
        let params;
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
        if (isDynamic && page !== nonLocalizedPath) {
            const normalizedPage = isAppDir ? (0, _apppaths.normalizeAppPath)(page) : page;
            params = (0, _getparams.getParams)(normalizedPage, updatedPath);
            if (params) {
                query = {
                    ...query,
                    ...params
                };
            }
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
        if (trailingSlash && !((_req_url = req.url) == null ? void 0 : _req_url.endsWith("/"))) {
            req.url += "/";
        }
        if (locale && buildExport && input.renderOpts.domainLocales && input.renderOpts.domainLocales.some((dl)=>{
            var _dl_locales;
            return dl.defaultLocale === locale || ((_dl_locales = dl.locales) == null ? void 0 : _dl_locales.includes(locale || ""));
        })) {
            (0, _requestmeta.addRequestMeta)(req, "isLocaleDomain", true);
        }
        envConfig.setConfig({
            serverRuntimeConfig,
            publicRuntimeConfig: input.renderOpts.runtimeConfig
        });
        const getHtmlFilename = (p)=>subFolders ? `${p}${_path.sep}index.html` : `${p}.html`;
        let htmlFilename = getHtmlFilename(filePath);
        // dynamic routes can provide invalid extensions e.g. /blog/[...slug] returns an
        // extension of `.slug]`
        const pageExt = isDynamic || isAppDir ? "" : (0, _path.extname)(page);
        const pathExt = isDynamic || isAppDir ? "" : (0, _path.extname)(path);
        // force output 404.html for backwards compat
        if (path === "/404.html") {
            htmlFilename = path;
        } else if (pageExt !== pathExt && pathExt !== "") {
            const isBuiltinPaths = [
                "/500",
                "/404"
            ].some((p)=>p === path || p === path + ".html");
            // If the ssg path has .html extension, and it's not builtin paths, use it directly
            // Otherwise, use that as the filename instead
            const isHtmlExtPath = !isBuiltinPaths && path.endsWith(".html");
            htmlFilename = isHtmlExtPath ? getHtmlFilename(path) : path;
        } else if (path === "/") {
            // If the path is the root, just use index.html
            htmlFilename = "index.html";
        }
        const baseDir = (0, _path.join)(outDir, (0, _path.dirname)(htmlFilename));
        let htmlFilepath = (0, _path.join)(outDir, htmlFilename);
        await _promises.default.mkdir(baseDir, {
            recursive: true
        });
        // If the fetch cache was enabled, we need to create an incremental
        // cache instance for this page.
        const incrementalCache = isAppDir && fetchCache ? await (0, _createincrementalcache.createIncrementalCache)({
            cacheHandler,
            cacheMaxMemorySize,
            fetchCacheKeyPrefix,
            distDir,
            dir,
            enabledDirectories,
            // PPR is not available for Pages.
            experimental: {
                ppr: false
            },
            // skip writing to disk in minimal mode for now, pending some
            // changes to better support it
            flushToDisk: !_ciinfo.hasNextSupport
        }) : undefined;
        // Handle App Routes.
        if (isAppDir && (0, _isapprouteroute.isAppRouteRoute)(page)) {
            return await (0, _approute.exportAppRoute)(req, res, params, page, incrementalCache, distDir, htmlFilepath, fileWriter);
        }
        const components = await (0, _loadcomponents.loadComponents)({
            distDir,
            page,
            isAppPath: isAppDir
        });
        const renderOpts = {
            ...components,
            ...input.renderOpts,
            ampPath: renderAmpPath,
            params,
            optimizeFonts,
            optimizeCss,
            disableOptimizedLoading,
            fontManifest: optimizeFonts ? (0, _require.requireFontManifest)(distDir) : undefined,
            locale,
            supportsDynamicHTML: false,
            originalPathname: page
        };
        if (_ciinfo.hasNextSupport) {
            renderOpts.isRevalidate = true;
        }
        // Handle App Pages
        if (isAppDir) {
            // Set the incremental cache on the renderOpts, that's how app page's
            // consume it.
            renderOpts.incrementalCache = incrementalCache;
            return await (0, _apppage.exportAppPage)(req, res, page, path, pathname, query, renderOpts, htmlFilepath, debugOutput, isDynamicError, fileWriter);
        }
        return await (0, _pages.exportPages)(req, res, path, page, query, htmlFilepath, htmlFilename, ampPath, subFolders, outDir, ampValidatorPath, pagesDataDir, buildExport, isDynamic, hasOrigQueryValues, renderOpts, components, fileWriter);
    } catch (err) {
        console.error(`\nError occurred prerendering page "${path}". Read more: https://nextjs.org/docs/messages/prerender-error\n`);
        if (!(0, _bailouttocsr.isBailoutToCSRError)(err)) {
            console.error((0, _iserror.default)(err) && err.stack ? err.stack : err);
        }
        return {
            error: true
        };
    }
}
async function exportPage(input) {
    // Configure the http agent.
    (0, _setuphttpagentenv.setHttpClientAndAgentOptions)({
        httpAgentOptions: input.httpAgentOptions
    });
    const files = [];
    const baseFileWriter = async (type, path, content, encodingOptions = "utf-8")=>{
        await _promises.default.mkdir((0, _path.dirname)(path), {
            recursive: true
        });
        await _promises.default.writeFile(path, content, encodingOptions);
        files.push({
            type,
            path
        });
    };
    const exportPageSpan = (0, _trace.trace)("export-page-worker", input.parentSpanId);
    const start = Date.now();
    const turborepoAccessTraceResult = new _turborepoaccesstrace.TurborepoAccessTraceResult();
    // Export the page.
    const result = await exportPageSpan.traceAsyncFn(()=>(0, _turborepoaccesstrace.turborepoTraceAccess)(()=>exportPageImpl(input, baseFileWriter), turborepoAccessTraceResult));
    // If there was no result, then we can exit early.
    if (!result) return;
    // If there was an error, then we can exit early.
    if ("error" in result) {
        return {
            error: result.error,
            duration: Date.now() - start,
            files: []
        };
    }
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
        turborepoAccessTraceResult: turborepoAccessTraceResult.serialize()
    };
}
process.on("unhandledRejection", (err)=>{
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
process.on("rejectionHandled", ()=>{
// It is ok to await a Promise late in Next.js as it allows for better
// prefetching patterns to avoid waterfalls. We ignore logging these.
// We should've already errored in anyway unhandledRejection.
});

//# sourceMappingURL=worker.js.map