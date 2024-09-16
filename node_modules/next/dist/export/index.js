"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ExportError: null,
    default: null,
    exportAppImpl: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ExportError: function() {
        return ExportError;
    },
    default: function() {
        return exportApp;
    },
    exportAppImpl: function() {
        return exportAppImpl;
    }
});
const _picocolors = require("../lib/picocolors");
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _fs = require("fs");
require("../server/require-hook");
const _worker = require("../lib/worker");
const _path = require("path");
const _index = require("../build/output/index");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _constants = require("../lib/constants");
const _recursivecopy = require("../lib/recursive-copy");
const _constants1 = require("../shared/lib/constants");
const _config = /*#__PURE__*/ _interop_require_default(require("../server/config"));
const _events = require("../telemetry/events");
const _ciinfo = require("../telemetry/ci-info");
const _storage = require("../telemetry/storage");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _denormalizepagepath = require("../shared/lib/page-path/denormalize-page-path");
const _env = require("@next/env");
const _isapiroute = require("../lib/is-api-route");
const _require = require("../server/require");
const _isapprouteroute = require("../lib/is-app-route-route");
const _isapppageroute = require("../lib/is-app-page-route");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
const _needsexperimentalreact = require("../lib/needs-experimental-react");
const _formatmanifest = require("../build/manifests/formatter/format-manifest");
const _patchfetch = require("../server/lib/patch-fetch");
const _turborepoaccesstrace = require("../build/turborepo-access-trace");
const _progress = require("../build/progress");
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
class ExportError extends Error {
    constructor(...args){
        super(...args);
        this.code = "NEXT_EXPORT_ERROR";
    }
}
function setupWorkers(options, nextConfig) {
    if (options.exportPageWorker) {
        return {
            pages: options.exportPageWorker,
            app: options.exportAppPageWorker,
            end: options.endWorker || (()=>Promise.resolve())
        };
    }
    const threads = options.threads || nextConfig.experimental.cpus;
    if (!options.silent && !options.buildExport) {
        _log.info(`Launching ${threads} workers`);
    }
    const timeout = (nextConfig == null ? void 0 : nextConfig.staticPageGenerationTimeout) || 0;
    let infoPrinted = false;
    const worker = new _worker.Worker(require.resolve("./worker"), {
        timeout: timeout * 1000,
        onRestart: (_method, [{ path }], attempts)=>{
            if (attempts >= 3) {
                throw new ExportError(`Static page generation for ${path} is still timing out after 3 attempts. See more info here https://nextjs.org/docs/messages/static-page-generation-timeout`);
            }
            _log.warn(`Restarted static page generation for ${path} because it took more than ${timeout} seconds`);
            if (!infoPrinted) {
                _log.warn("See more info here https://nextjs.org/docs/messages/static-page-generation-timeout");
                infoPrinted = true;
            }
        },
        maxRetries: 0,
        numWorkers: threads,
        enableWorkerThreads: nextConfig.experimental.workerThreads,
        exposedMethods: [
            "default"
        ]
    });
    return {
        pages: worker.default,
        end: async ()=>{
            await worker.end();
        }
    };
}
async function exportAppImpl(dir, options, span) {
    var _nextConfig_amp, _nextConfig_experimental_amp, _nextConfig_experimental_amp1;
    dir = (0, _path.resolve)(dir);
    // attempt to load global env values so they are available in next.config.js
    span.traceChild("load-dotenv").traceFn(()=>(0, _env.loadEnvConfig)(dir, false, _log));
    const { enabledDirectories } = options;
    const nextConfig = options.nextConfig || await span.traceChild("load-next-config").traceAsyncFn(()=>(0, _config.default)(_constants1.PHASE_EXPORT, dir));
    const distDir = (0, _path.join)(dir, nextConfig.distDir);
    const telemetry = options.buildExport ? null : new _storage.Telemetry({
        distDir
    });
    if (telemetry) {
        telemetry.record((0, _events.eventCliSession)(distDir, nextConfig, {
            webpackVersion: null,
            cliCommand: "export",
            isSrcDir: null,
            hasNowJson: !!await (0, _findup.default)("now.json", {
                cwd: dir
            }),
            isCustomServer: null,
            turboFlag: false,
            pagesDir: null,
            appDir: null
        }));
    }
    const subFolders = nextConfig.trailingSlash && !options.buildExport;
    if (!options.silent && !options.buildExport) {
        _log.info(`using build directory: ${distDir}`);
    }
    const buildIdFile = (0, _path.join)(distDir, _constants1.BUILD_ID_FILE);
    if (!(0, _fs.existsSync)(buildIdFile)) {
        throw new ExportError(`Could not find a production build in the '${distDir}' directory. Try building your app with 'next build' before starting the static export. https://nextjs.org/docs/messages/next-export-no-build-id`);
    }
    const customRoutes = [
        "rewrites",
        "redirects",
        "headers"
    ].filter((config)=>typeof nextConfig[config] === "function");
    if (!_ciinfo.hasNextSupport && !options.buildExport && customRoutes.length > 0) {
        _log.warn(`rewrites, redirects, and headers are not applied when exporting your application, detected (${customRoutes.join(", ")}). See more info here: https://nextjs.org/docs/messages/export-no-custom-routes`);
    }
    const buildId = await _fs.promises.readFile(buildIdFile, "utf8");
    const pagesManifest = !options.pages && require((0, _path.join)(distDir, _constants1.SERVER_DIRECTORY, _constants1.PAGES_MANIFEST));
    let prerenderManifest;
    try {
        prerenderManifest = require((0, _path.join)(distDir, _constants1.PRERENDER_MANIFEST));
    } catch  {}
    let appRoutePathManifest;
    try {
        appRoutePathManifest = require((0, _path.join)(distDir, _constants1.APP_PATH_ROUTES_MANIFEST));
    } catch (err) {
        if ((0, _iserror.default)(err) && (err.code === "ENOENT" || err.code === "MODULE_NOT_FOUND")) {
            // the manifest doesn't exist which will happen when using
            // "pages" dir instead of "app" dir.
            appRoutePathManifest = undefined;
        } else {
            // the manifest is malformed (invalid json)
            throw err;
        }
    }
    const excludedPrerenderRoutes = new Set();
    const pages = options.pages || Object.keys(pagesManifest);
    const defaultPathMap = {};
    let hasApiRoutes = false;
    for (const page of pages){
        // _document and _app are not real pages
        // _error is exported as 404.html later on
        // API Routes are Node.js functions
        if ((0, _isapiroute.isAPIRoute)(page)) {
            hasApiRoutes = true;
            continue;
        }
        if (page === "/_document" || page === "/_app" || page === "/_error") {
            continue;
        }
        // iSSG pages that are dynamic should not export templated version by
        // default. In most cases, this would never work. There is no server that
        // could run `getStaticProps`. If users make their page work lazily, they
        // can manually add it to the `exportPathMap`.
        if (prerenderManifest == null ? void 0 : prerenderManifest.dynamicRoutes[page]) {
            excludedPrerenderRoutes.add(page);
            continue;
        }
        defaultPathMap[page] = {
            page
        };
    }
    const mapAppRouteToPage = new Map();
    if (!options.buildExport && appRoutePathManifest) {
        for (const [pageName, routePath] of Object.entries(appRoutePathManifest)){
            mapAppRouteToPage.set(routePath, pageName);
            if ((0, _isapppageroute.isAppPageRoute)(pageName) && !(prerenderManifest == null ? void 0 : prerenderManifest.routes[routePath]) && !(prerenderManifest == null ? void 0 : prerenderManifest.dynamicRoutes[routePath])) {
                defaultPathMap[routePath] = {
                    page: pageName,
                    _isAppDir: true
                };
            }
        }
    }
    // Initialize the output directory
    const outDir = options.outdir;
    if (outDir === (0, _path.join)(dir, "public")) {
        throw new ExportError(`The 'public' directory is reserved in Next.js and can not be used as the export out directory. https://nextjs.org/docs/messages/can-not-output-to-public`);
    }
    if (outDir === (0, _path.join)(dir, "static")) {
        throw new ExportError(`The 'static' directory is reserved in Next.js and can not be used as the export out directory. https://nextjs.org/docs/messages/can-not-output-to-static`);
    }
    await _fs.promises.rm(outDir, {
        recursive: true,
        force: true
    });
    await _fs.promises.mkdir((0, _path.join)(outDir, "_next", buildId), {
        recursive: true
    });
    await _fs.promises.writeFile((0, _path.join)(distDir, _constants1.EXPORT_DETAIL), (0, _formatmanifest.formatManifest)({
        version: 1,
        outDirectory: outDir,
        success: false
    }), "utf8");
    // Copy static directory
    if (!options.buildExport && (0, _fs.existsSync)((0, _path.join)(dir, "static"))) {
        if (!options.silent) {
            _log.info('Copying "static" directory');
        }
        await span.traceChild("copy-static-directory").traceAsyncFn(()=>(0, _recursivecopy.recursiveCopy)((0, _path.join)(dir, "static"), (0, _path.join)(outDir, "static")));
    }
    // Copy .next/static directory
    if (!options.buildExport && (0, _fs.existsSync)((0, _path.join)(distDir, _constants1.CLIENT_STATIC_FILES_PATH))) {
        if (!options.silent) {
            _log.info('Copying "static build" directory');
        }
        await span.traceChild("copy-next-static-directory").traceAsyncFn(()=>(0, _recursivecopy.recursiveCopy)((0, _path.join)(distDir, _constants1.CLIENT_STATIC_FILES_PATH), (0, _path.join)(outDir, "_next", _constants1.CLIENT_STATIC_FILES_PATH)));
    }
    // Get the exportPathMap from the config file
    if (typeof nextConfig.exportPathMap !== "function") {
        nextConfig.exportPathMap = async (defaultMap)=>{
            return defaultMap;
        };
    }
    const { i18n, images: { loader = "default", unoptimized } } = nextConfig;
    if (i18n && !options.buildExport) {
        throw new ExportError(`i18n support is not compatible with next export. See here for more info on deploying: https://nextjs.org/docs/messages/export-no-custom-routes`);
    }
    if (!options.buildExport) {
        const { isNextImageImported } = await span.traceChild("is-next-image-imported").traceAsyncFn(()=>_fs.promises.readFile((0, _path.join)(distDir, _constants1.EXPORT_MARKER), "utf8").then((text)=>JSON.parse(text)).catch(()=>({})));
        if (isNextImageImported && loader === "default" && !unoptimized && !_ciinfo.hasNextSupport) {
            throw new ExportError(`Image Optimization using the default loader is not compatible with export.
  Possible solutions:
    - Use \`next start\` to run a server, which includes the Image Optimization API.
    - Configure \`images.unoptimized = true\` in \`next.config.js\` to disable the Image Optimization API.
  Read more: https://nextjs.org/docs/messages/export-image-api`);
        }
    }
    let serverActionsManifest;
    if (enabledDirectories.app) {
        serverActionsManifest = require((0, _path.join)(distDir, _constants1.SERVER_DIRECTORY, _constants1.SERVER_REFERENCE_MANIFEST + ".json"));
        if (nextConfig.output === "export") {
            if (Object.keys(serverActionsManifest.node).length > 0 || Object.keys(serverActionsManifest.edge).length > 0) {
                throw new ExportError(`Server Actions are not supported with static export.`);
            }
        }
    }
    // Start the rendering process
    const renderOpts = {
        previewProps: prerenderManifest == null ? void 0 : prerenderManifest.preview,
        buildId,
        nextExport: true,
        assetPrefix: nextConfig.assetPrefix.replace(/\/$/, ""),
        distDir,
        dev: false,
        basePath: nextConfig.basePath,
        trailingSlash: nextConfig.trailingSlash,
        canonicalBase: ((_nextConfig_amp = nextConfig.amp) == null ? void 0 : _nextConfig_amp.canonicalBase) || "",
        ampSkipValidation: ((_nextConfig_experimental_amp = nextConfig.experimental.amp) == null ? void 0 : _nextConfig_experimental_amp.skipValidation) || false,
        ampOptimizerConfig: ((_nextConfig_experimental_amp1 = nextConfig.experimental.amp) == null ? void 0 : _nextConfig_experimental_amp1.optimizer) || undefined,
        locales: i18n == null ? void 0 : i18n.locales,
        locale: i18n == null ? void 0 : i18n.defaultLocale,
        defaultLocale: i18n == null ? void 0 : i18n.defaultLocale,
        domainLocales: i18n == null ? void 0 : i18n.domains,
        disableOptimizedLoading: nextConfig.experimental.disableOptimizedLoading,
        // Exported pages do not currently support dynamic HTML.
        supportsDynamicHTML: false,
        crossOrigin: nextConfig.crossOrigin,
        optimizeCss: nextConfig.experimental.optimizeCss,
        nextConfigOutput: nextConfig.output,
        nextScriptWorkers: nextConfig.experimental.nextScriptWorkers,
        optimizeFonts: nextConfig.optimizeFonts,
        largePageDataBytes: nextConfig.experimental.largePageDataBytes,
        serverActions: nextConfig.experimental.serverActions,
        serverComponents: enabledDirectories.app,
        nextFontManifest: require((0, _path.join)(distDir, "server", `${_constants1.NEXT_FONT_MANIFEST}.json`)),
        images: nextConfig.images,
        ...enabledDirectories.app ? {
            serverActionsManifest
        } : {},
        strictNextHead: !!nextConfig.experimental.strictNextHead,
        deploymentId: nextConfig.deploymentId,
        experimental: {
            ppr: nextConfig.experimental.ppr === true,
            missingSuspenseWithCSRBailout: nextConfig.experimental.missingSuspenseWithCSRBailout === true,
            swrDelta: nextConfig.experimental.swrDelta
        }
    };
    const { serverRuntimeConfig, publicRuntimeConfig } = nextConfig;
    if (Object.keys(publicRuntimeConfig).length > 0) {
        renderOpts.runtimeConfig = publicRuntimeConfig;
    }
    globalThis.__NEXT_DATA__ = {
        nextExport: true
    };
    const exportPathMap = await span.traceChild("run-export-path-map").traceAsyncFn(async ()=>{
        const exportMap = await nextConfig.exportPathMap(defaultPathMap, {
            dev: false,
            dir,
            outDir,
            distDir,
            buildId
        });
        return exportMap;
    });
    // only add missing 404 page when `buildExport` is false
    if (!options.buildExport) {
        // only add missing /404 if not specified in `exportPathMap`
        if (!exportPathMap["/404"]) {
            exportPathMap["/404"] = {
                page: "/_error"
            };
        }
        /**
     * exports 404.html for backwards compat
     * E.g. GitHub Pages, GitLab Pages, Cloudflare Pages, Netlify
     */ if (!exportPathMap["/404.html"]) {
            // alias /404.html to /404 to be compatible with custom 404 / _error page
            exportPathMap["/404.html"] = exportPathMap["/404"];
        }
    }
    // make sure to prevent duplicates
    const exportPaths = [
        ...new Set(Object.keys(exportPathMap).map((path)=>(0, _denormalizepagepath.denormalizePagePath)((0, _normalizepagepath.normalizePagePath)(path))))
    ];
    const filteredPaths = exportPaths.filter((route)=>exportPathMap[route]._isAppDir || // Remove API routes
        !(0, _isapiroute.isAPIRoute)(exportPathMap[route].page));
    if (filteredPaths.length !== exportPaths.length) {
        hasApiRoutes = true;
    }
    if (filteredPaths.length === 0) {
        return null;
    }
    if (prerenderManifest && !options.buildExport) {
        const fallbackEnabledPages = new Set();
        for (const path of Object.keys(exportPathMap)){
            const page = exportPathMap[path].page;
            const prerenderInfo = prerenderManifest.dynamicRoutes[page];
            if (prerenderInfo && prerenderInfo.fallback !== false) {
                fallbackEnabledPages.add(page);
            }
        }
        if (fallbackEnabledPages.size > 0) {
            throw new ExportError(`Found pages with \`fallback\` enabled:\n${[
                ...fallbackEnabledPages
            ].join("\n")}\n${_constants.SSG_FALLBACK_EXPORT_ERROR}\n`);
        }
    }
    let hasMiddleware = false;
    if (!options.buildExport) {
        try {
            const middlewareManifest = require((0, _path.join)(distDir, _constants1.SERVER_DIRECTORY, _constants1.MIDDLEWARE_MANIFEST));
            hasMiddleware = Object.keys(middlewareManifest.middleware).length > 0;
        } catch  {}
        // Warn if the user defines a path for an API page
        if (hasApiRoutes || hasMiddleware) {
            if (nextConfig.output === "export") {
                _log.warn((0, _picocolors.yellow)(`Statically exporting a Next.js application via \`next export\` disables API routes and middleware.`) + `\n` + (0, _picocolors.yellow)(`This command is meant for static-only hosts, and is` + " " + (0, _picocolors.bold)(`not necessary to make your application static.`)) + `\n` + (0, _picocolors.yellow)(`Pages in your application without server-side data dependencies will be automatically statically exported by \`next build\`, including pages powered by \`getStaticProps\`.`) + `\n` + (0, _picocolors.yellow)(`Learn more: https://nextjs.org/docs/messages/api-routes-static-export`));
            }
        }
    }
    const progress = !options.silent && (0, _progress.createProgress)(filteredPaths.length, options.statusMessage || "Exporting");
    const pagesDataDir = options.buildExport ? outDir : (0, _path.join)(outDir, "_next/data", buildId);
    const ampValidations = {};
    const publicDir = (0, _path.join)(dir, _constants1.CLIENT_PUBLIC_FILES_PATH);
    // Copy public directory
    if (!options.buildExport && (0, _fs.existsSync)(publicDir)) {
        if (!options.silent) {
            _log.info('Copying "public" directory');
        }
        await span.traceChild("copy-public-directory").traceAsyncFn(()=>(0, _recursivecopy.recursiveCopy)(publicDir, outDir, {
                filter (path) {
                    // Exclude paths used by pages
                    return !exportPathMap[path];
                }
            }));
    }
    const workers = setupWorkers(options, nextConfig);
    const results = await Promise.all(filteredPaths.map(async (path)=>{
        const pathMap = exportPathMap[path];
        const exportPage = workers[pathMap._isAppDir ? "app" : "pages"];
        if (!exportPage) {
            throw new Error("Invariant: Undefined export worker for app dir, this is a bug in Next.js.");
        }
        const pageExportSpan = span.traceChild("export-page");
        pageExportSpan.setAttribute("path", path);
        const result = await pageExportSpan.traceAsyncFn(async ()=>{
            var _nextConfig_experimental_amp;
            return await exportPage({
                dir,
                path,
                pathMap,
                distDir,
                outDir,
                pagesDataDir,
                renderOpts,
                ampValidatorPath: ((_nextConfig_experimental_amp = nextConfig.experimental.amp) == null ? void 0 : _nextConfig_experimental_amp.validator) || undefined,
                trailingSlash: nextConfig.trailingSlash,
                serverRuntimeConfig,
                subFolders,
                buildExport: options.buildExport,
                optimizeFonts: nextConfig.optimizeFonts,
                optimizeCss: nextConfig.experimental.optimizeCss,
                disableOptimizedLoading: nextConfig.experimental.disableOptimizedLoading,
                parentSpanId: pageExportSpan.getId(),
                httpAgentOptions: nextConfig.httpAgentOptions,
                debugOutput: options.debugOutput,
                cacheMaxMemorySize: nextConfig.cacheMaxMemorySize,
                fetchCache: true,
                fetchCacheKeyPrefix: nextConfig.experimental.fetchCacheKeyPrefix,
                cacheHandler: nextConfig.cacheHandler,
                enableExperimentalReact: (0, _needsexperimentalreact.needsExperimentalReact)(nextConfig),
                enabledDirectories
            });
        });
        if (nextConfig.experimental.prerenderEarlyExit) {
            if (result && "error" in result) {
                throw new Error(`Export encountered an error on ${path}, exiting due to prerenderEarlyExit: true being set`);
            }
        }
        if (progress) progress();
        return {
            result,
            path
        };
    }));
    const errorPaths = [];
    let renderError = false;
    let hadValidationError = false;
    const collector = {
        byPath: new Map(),
        byPage: new Map(),
        ssgNotFoundPaths: new Set(),
        turborepoAccessTraceResults: new Map()
    };
    for (const { result, path } of results){
        if (!result) continue;
        const { page } = exportPathMap[path];
        if (result.turborepoAccessTraceResult) {
            var _collector_turborepoAccessTraceResults;
            (_collector_turborepoAccessTraceResults = collector.turborepoAccessTraceResults) == null ? void 0 : _collector_turborepoAccessTraceResults.set(path, _turborepoaccesstrace.TurborepoAccessTraceResult.fromSerialized(result.turborepoAccessTraceResult));
        }
        // Capture any render errors.
        if ("error" in result) {
            renderError = true;
            errorPaths.push(page !== path ? `${page}: ${path}` : path);
            continue;
        }
        // Capture any amp validations.
        if (result.ampValidations) {
            for (const validation of result.ampValidations){
                ampValidations[validation.page] = validation.result;
                hadValidationError ||= validation.result.errors.length > 0;
            }
        }
        if (options.buildExport) {
            // Update path info by path.
            const info = collector.byPath.get(path) ?? {};
            if (typeof result.revalidate !== "undefined") {
                info.revalidate = (0, _patchfetch.validateRevalidate)(result.revalidate, path);
            }
            if (typeof result.metadata !== "undefined") {
                info.metadata = result.metadata;
            }
            if (typeof result.hasEmptyPrelude !== "undefined") {
                info.hasEmptyPrelude = result.hasEmptyPrelude;
            }
            if (typeof result.hasPostponed !== "undefined") {
                info.hasPostponed = result.hasPostponed;
            }
            collector.byPath.set(path, info);
            // Update not found.
            if (result.ssgNotFound === true) {
                collector.ssgNotFoundPaths.add(path);
            }
            // Update durations.
            const durations = collector.byPage.get(page) ?? {
                durationsByPath: new Map()
            };
            durations.durationsByPath.set(path, result.duration);
            collector.byPage.set(page, durations);
        }
    }
    const endWorkerPromise = workers.end();
    // Export mode provide static outputs that are not compatible with PPR mode.
    if (!options.buildExport && nextConfig.experimental.ppr) {
        // TODO: add message
        throw new Error("Invariant: PPR cannot be enabled in export mode");
    }
    // copy prerendered routes to outDir
    if (!options.buildExport && prerenderManifest) {
        await Promise.all(Object.keys(prerenderManifest.routes).map(async (route)=>{
            const { srcRoute } = prerenderManifest.routes[route];
            const appPageName = mapAppRouteToPage.get(srcRoute || "");
            const pageName = appPageName || srcRoute || route;
            const isAppPath = Boolean(appPageName);
            const isAppRouteHandler = appPageName && (0, _isapprouteroute.isAppRouteRoute)(appPageName);
            // returning notFound: true from getStaticProps will not
            // output html/json files during the build
            if (prerenderManifest.notFoundRoutes.includes(route)) {
                return;
            }
            route = (0, _normalizepagepath.normalizePagePath)(route);
            const pagePath = (0, _require.getPagePath)(pageName, distDir, undefined, isAppPath);
            const distPagesDir = (0, _path.join)(pagePath, // strip leading / and then recurse number of nested dirs
            // to place from base folder
            pageName.slice(1).split("/").map(()=>"..").join("/"));
            const orig = (0, _path.join)(distPagesDir, route);
            const handlerSrc = `${orig}.body`;
            const handlerDest = (0, _path.join)(outDir, route);
            if (isAppRouteHandler && (0, _fs.existsSync)(handlerSrc)) {
                await _fs.promises.mkdir((0, _path.dirname)(handlerDest), {
                    recursive: true
                });
                await _fs.promises.copyFile(handlerSrc, handlerDest);
                return;
            }
            const htmlDest = (0, _path.join)(outDir, `${route}${subFolders && route !== "/index" ? `${_path.sep}index` : ""}.html`);
            const ampHtmlDest = (0, _path.join)(outDir, `${route}.amp${subFolders ? `${_path.sep}index` : ""}.html`);
            const jsonDest = isAppPath ? (0, _path.join)(outDir, `${route}${subFolders && route !== "/index" ? `${_path.sep}index` : ""}.txt`) : (0, _path.join)(pagesDataDir, `${route}.json`);
            await _fs.promises.mkdir((0, _path.dirname)(htmlDest), {
                recursive: true
            });
            await _fs.promises.mkdir((0, _path.dirname)(jsonDest), {
                recursive: true
            });
            const htmlSrc = `${orig}.html`;
            const jsonSrc = `${orig}${isAppPath ? _constants.RSC_SUFFIX : ".json"}`;
            await _fs.promises.copyFile(htmlSrc, htmlDest);
            await _fs.promises.copyFile(jsonSrc, jsonDest);
            if ((0, _fs.existsSync)(`${orig}.amp.html`)) {
                await _fs.promises.mkdir((0, _path.dirname)(ampHtmlDest), {
                    recursive: true
                });
                await _fs.promises.copyFile(`${orig}.amp.html`, ampHtmlDest);
            }
        }));
    }
    if (Object.keys(ampValidations).length) {
        console.log((0, _index.formatAmpMessages)(ampValidations));
    }
    if (hadValidationError) {
        throw new ExportError(`AMP Validation caused the export to fail. https://nextjs.org/docs/messages/amp-export-validation`);
    }
    if (renderError) {
        throw new ExportError(`Export encountered errors on following paths:\n\t${errorPaths.sort().join("\n	")}`);
    }
    await _fs.promises.writeFile((0, _path.join)(distDir, _constants1.EXPORT_DETAIL), (0, _formatmanifest.formatManifest)({
        version: 1,
        outDirectory: outDir,
        success: true
    }), "utf8");
    if (telemetry) {
        await telemetry.flush();
    }
    await endWorkerPromise;
    return collector;
}
async function exportApp(dir, options, span) {
    const nextExportSpan = span.traceChild("next-export");
    return nextExportSpan.traceAsyncFn(async ()=>{
        return await exportAppImpl(dir, options, nextExportSpan);
    });
}

//# sourceMappingURL=index.js.map