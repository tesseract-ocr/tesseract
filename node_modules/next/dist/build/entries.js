"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createEntrypoints: null,
    createPagesMapping: null,
    finalizeEntrypoint: null,
    getAppEntry: null,
    getClientEntry: null,
    getEdgeServerEntry: null,
    getInstrumentationEntry: null,
    getPageFilePath: null,
    getPageFromPath: null,
    getStaticInfoIncludingLayouts: null,
    runDependingOnPageType: null,
    sortByPageExts: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createEntrypoints: function() {
        return createEntrypoints;
    },
    createPagesMapping: function() {
        return createPagesMapping;
    },
    finalizeEntrypoint: function() {
        return finalizeEntrypoint;
    },
    getAppEntry: function() {
        return getAppEntry;
    },
    getClientEntry: function() {
        return getClientEntry;
    },
    getEdgeServerEntry: function() {
        return getEdgeServerEntry;
    },
    getInstrumentationEntry: function() {
        return getInstrumentationEntry;
    },
    getPageFilePath: function() {
        return getPageFilePath;
    },
    getPageFromPath: function() {
        return getPageFromPath;
    },
    getStaticInfoIncludingLayouts: function() {
        return getStaticInfoIncludingLayouts;
    },
    runDependingOnPageType: function() {
        return runDependingOnPageType;
    },
    sortByPageExts: function() {
        return sortByPageExts;
    }
});
const _path = require("path");
const _querystring = require("querystring");
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _constants = require("../lib/constants");
const _isapiroute = require("../lib/is-api-route");
const _isedgeruntime = require("../lib/is-edge-runtime");
const _constants1 = require("../shared/lib/constants");
const _utils = require("./utils");
const _getpagestaticinfo = require("./analysis/get-page-static-info");
const _normalizepathsep = require("../shared/lib/page-path/normalize-path-sep");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _nextmiddlewareloader = require("./webpack/loaders/next-middleware-loader");
const _isapprouteroute = require("../lib/is-app-route-route");
const _getmetadataroute = require("../lib/metadata/get-metadata-route");
const _nextrouteloader = require("./webpack/loaders/next-route-loader");
const _isinternalcomponent = require("../lib/is-internal-component");
const _ismetadataroute = require("../lib/metadata/is-metadata-route");
const _routekind = require("../server/route-kind");
const _utils1 = require("./webpack/loaders/utils");
const _normalizecatchallroutes = require("./normalize-catchall-routes");
const _pagetypes = require("../lib/page-types");
const _isapppageroute = require("../lib/is-app-page-route");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function sortByPageExts(pageExtensions) {
    return (a, b)=>{
        // prioritize entries according to pageExtensions order
        // for consistency as fs order can differ across systems
        // NOTE: this is reversed so preferred comes last and
        // overrides prior
        const aExt = (0, _path.extname)(a);
        const bExt = (0, _path.extname)(b);
        const aNoExt = a.substring(0, a.length - aExt.length);
        const bNoExt = a.substring(0, b.length - bExt.length);
        if (aNoExt !== bNoExt) return 0;
        // find extension index (skip '.' as pageExtensions doesn't have it)
        const aExtIndex = pageExtensions.indexOf(aExt.substring(1));
        const bExtIndex = pageExtensions.indexOf(bExt.substring(1));
        return bExtIndex - aExtIndex;
    };
}
async function getStaticInfoIncludingLayouts({ isInsideAppDir, pageExtensions, pageFilePath, appDir, config: nextConfig, isDev, page }) {
    // TODO: sync types for pages: PAGE_TYPES, ROUTER_TYPE, 'app' | 'pages', etc.
    const pageType = isInsideAppDir ? _pagetypes.PAGE_TYPES.APP : _pagetypes.PAGE_TYPES.PAGES;
    const pageStaticInfo = await (0, _getpagestaticinfo.getPageStaticInfo)({
        nextConfig,
        pageFilePath,
        isDev,
        page,
        pageType
    });
    if (pageStaticInfo.type === _pagetypes.PAGE_TYPES.PAGES || !appDir) {
        return pageStaticInfo;
    }
    const segments = [
        pageStaticInfo
    ];
    // inherit from layout files only if it's a page route
    if ((0, _isapppageroute.isAppPageRoute)(page)) {
        const layoutFiles = [];
        const potentialLayoutFiles = pageExtensions.map((ext)=>'layout.' + ext);
        let dir = (0, _path.dirname)(pageFilePath);
        // Uses startsWith to not include directories further up.
        while(dir.startsWith(appDir)){
            for (const potentialLayoutFile of potentialLayoutFiles){
                const layoutFile = (0, _path.join)(dir, potentialLayoutFile);
                if (!_fs.default.existsSync(layoutFile)) {
                    continue;
                }
                layoutFiles.push(layoutFile);
            }
            // Walk up the directory tree
            dir = (0, _path.join)(dir, '..');
        }
        for (const layoutFile of layoutFiles){
            const layoutStaticInfo = await (0, _getpagestaticinfo.getAppPageStaticInfo)({
                nextConfig,
                pageFilePath: layoutFile,
                isDev,
                page,
                pageType: isInsideAppDir ? _pagetypes.PAGE_TYPES.APP : _pagetypes.PAGE_TYPES.PAGES
            });
            segments.unshift(layoutStaticInfo);
        }
    }
    const config = (0, _utils.reduceAppConfig)(segments);
    return {
        ...pageStaticInfo,
        config,
        runtime: config.runtime,
        preferredRegion: config.preferredRegion,
        maxDuration: config.maxDuration
    };
}
function getPageFromPath(pagePath, pageExtensions) {
    let page = (0, _normalizepathsep.normalizePathSep)(pagePath.replace(new RegExp(`\\.+(${pageExtensions.join('|')})$`), ''));
    page = page.replace(/\/index$/, '');
    return page === '' ? '/' : page;
}
function getPageFilePath({ absolutePagePath, pagesDir, appDir, rootDir }) {
    if (absolutePagePath.startsWith(_constants.PAGES_DIR_ALIAS) && pagesDir) {
        return absolutePagePath.replace(_constants.PAGES_DIR_ALIAS, pagesDir);
    }
    if (absolutePagePath.startsWith(_constants.APP_DIR_ALIAS) && appDir) {
        return absolutePagePath.replace(_constants.APP_DIR_ALIAS, appDir);
    }
    if (absolutePagePath.startsWith(_constants.ROOT_DIR_ALIAS)) {
        return absolutePagePath.replace(_constants.ROOT_DIR_ALIAS, rootDir);
    }
    return require.resolve(absolutePagePath);
}
async function createPagesMapping({ isDev, pageExtensions, pagePaths, pagesType, pagesDir, appDir }) {
    const isAppRoute = pagesType === 'app';
    const pages = {};
    const promises = pagePaths.map(async (pagePath)=>{
        // Do not process .d.ts files as routes
        if (pagePath.endsWith('.d.ts') && pageExtensions.includes('ts')) {
            return;
        }
        let pageKey = getPageFromPath(pagePath, pageExtensions);
        if (isAppRoute) {
            pageKey = pageKey.replace(/%5F/g, '_');
            if (pageKey === '/not-found') {
                pageKey = _constants1.UNDERSCORE_NOT_FOUND_ROUTE_ENTRY;
            }
        }
        const normalizedPath = (0, _normalizepathsep.normalizePathSep)((0, _path.join)(pagesType === 'pages' ? _constants.PAGES_DIR_ALIAS : pagesType === 'app' ? _constants.APP_DIR_ALIAS : _constants.ROOT_DIR_ALIAS, pagePath));
        let route = pagesType === 'app' ? (0, _getmetadataroute.normalizeMetadataRoute)(pageKey) : pageKey;
        if (pagesType === 'app' && (0, _ismetadataroute.isMetadataRouteFile)(pagePath, pageExtensions, true)) {
            const filePath = (0, _path.join)(appDir, pagePath);
            const staticInfo = await (0, _getpagestaticinfo.getPageStaticInfo)({
                nextConfig: {},
                pageFilePath: filePath,
                isDev,
                page: pageKey,
                pageType: pagesType
            });
            route = (0, _getmetadataroute.normalizeMetadataPageToRoute)(route, !!(staticInfo.generateImageMetadata || staticInfo.generateSitemaps));
        }
        pages[route] = normalizedPath;
    });
    await Promise.all(promises);
    switch(pagesType){
        case _pagetypes.PAGE_TYPES.ROOT:
            {
                return pages;
            }
        case _pagetypes.PAGE_TYPES.APP:
            {
                const hasAppPages = Object.keys(pages).some((page)=>page.endsWith('/page'));
                return {
                    // If there's any app pages existed, add a default not-found page.
                    // If there's any custom not-found page existed, it will override the default one.
                    ...hasAppPages && {
                        [_constants1.UNDERSCORE_NOT_FOUND_ROUTE_ENTRY]: 'next/dist/client/components/not-found-error'
                    },
                    ...pages
                };
            }
        case _pagetypes.PAGE_TYPES.PAGES:
            {
                if (isDev) {
                    delete pages['/_app'];
                    delete pages['/_error'];
                    delete pages['/_document'];
                }
                // In development we always alias these to allow Webpack to fallback to
                // the correct source file so that HMR can work properly when a file is
                // added or removed.
                const root = isDev && pagesDir ? _constants.PAGES_DIR_ALIAS : 'next/dist/pages';
                return {
                    '/_app': `${root}/_app`,
                    '/_error': `${root}/_error`,
                    '/_document': `${root}/_document`,
                    ...pages
                };
            }
        default:
            {
                return {};
            }
    }
}
function getEdgeServerEntry(opts) {
    var _opts_config_experimental_sri;
    if (opts.pagesType === 'app' && (0, _isapprouteroute.isAppRouteRoute)(opts.page) && opts.appDirLoader) {
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            appDirLoader: Buffer.from(opts.appDirLoader || '').toString('base64'),
            nextConfig: Buffer.from(JSON.stringify(opts.config)).toString('base64'),
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString('base64')
        };
        return {
            import: `next-edge-app-route-loader?${(0, _querystring.stringify)(loaderParams)}!`,
            layer: _constants.WEBPACK_LAYERS.reactServerComponents
        };
    }
    if ((0, _utils.isMiddlewareFile)(opts.page)) {
        var _opts_middleware;
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            rootDir: opts.rootDir,
            matchers: ((_opts_middleware = opts.middleware) == null ? void 0 : _opts_middleware.matchers) ? (0, _nextmiddlewareloader.encodeMatchers)(opts.middleware.matchers) : '',
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString('base64')
        };
        return `next-middleware-loader?${(0, _querystring.stringify)(loaderParams)}!`;
    }
    if ((0, _isapiroute.isAPIRoute)(opts.page)) {
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            rootDir: opts.rootDir,
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString('base64')
        };
        return `next-edge-function-loader?${(0, _querystring.stringify)(loaderParams)}!`;
    }
    const loaderParams = {
        absolute500Path: opts.pages['/500'] || '',
        absoluteAppPath: opts.pages['/_app'],
        absoluteDocumentPath: opts.pages['/_document'],
        absoluteErrorPath: opts.pages['/_error'],
        absolutePagePath: opts.absolutePagePath,
        dev: opts.isDev,
        isServerComponent: opts.isServerComponent,
        page: opts.page,
        stringifiedConfig: Buffer.from(JSON.stringify(opts.config)).toString('base64'),
        pagesType: opts.pagesType,
        appDirLoader: Buffer.from(opts.appDirLoader || '').toString('base64'),
        sriEnabled: !opts.isDev && !!((_opts_config_experimental_sri = opts.config.experimental.sri) == null ? void 0 : _opts_config_experimental_sri.algorithm),
        cacheHandler: opts.config.cacheHandler,
        preferredRegion: opts.preferredRegion,
        middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString('base64'),
        serverActions: opts.config.experimental.serverActions,
        cacheHandlers: JSON.stringify(opts.config.experimental.cacheHandlers || {})
    };
    return {
        import: `next-edge-ssr-loader?${JSON.stringify(loaderParams)}!`,
        // The Edge bundle includes the server in its entrypoint, so it has to
        // be in the SSR layer — we later convert the page request to the RSC layer
        // via a webpack rule.
        layer: opts.appDirLoader ? _constants.WEBPACK_LAYERS.serverSideRendering : undefined
    };
}
function getInstrumentationEntry(opts) {
    // the '../' is needed to make sure the file is not chunked
    const filename = `${opts.isEdgeServer ? 'edge-' : opts.isDev ? '' : '../'}${_constants.INSTRUMENTATION_HOOK_FILENAME}.js`;
    return {
        import: opts.absolutePagePath,
        filename,
        layer: _constants.WEBPACK_LAYERS.instrument
    };
}
function getAppEntry(opts) {
    return {
        import: `next-app-loader?${(0, _querystring.stringify)(opts)}!`,
        layer: _constants.WEBPACK_LAYERS.reactServerComponents
    };
}
function getClientEntry(opts) {
    const loaderOptions = {
        absolutePagePath: opts.absolutePagePath,
        page: opts.page
    };
    const pageLoader = `next-client-pages-loader?${(0, _querystring.stringify)(loaderOptions)}!`;
    // Make sure next/router is a dependency of _app or else chunk splitting
    // might cause the router to not be able to load causing hydration
    // to fail
    return opts.page === '/_app' ? [
        pageLoader,
        require.resolve('../client/router')
    ] : pageLoader;
}
function runDependingOnPageType(params) {
    if (params.pageType === _pagetypes.PAGE_TYPES.ROOT && (0, _utils.isInstrumentationHookFile)(params.page)) {
        params.onServer();
        params.onEdgeServer();
        return;
    }
    if ((0, _utils.isMiddlewareFile)(params.page)) {
        params.onEdgeServer();
        return;
    }
    if ((0, _isapiroute.isAPIRoute)(params.page)) {
        if ((0, _isedgeruntime.isEdgeRuntime)(params.pageRuntime)) {
            params.onEdgeServer();
            return;
        }
        params.onServer();
        return;
    }
    if (params.page === '/_document') {
        params.onServer();
        return;
    }
    if (params.page === '/_app' || params.page === '/_error' || params.page === '/404' || params.page === '/500') {
        params.onClient();
        params.onServer();
        return;
    }
    if ((0, _isedgeruntime.isEdgeRuntime)(params.pageRuntime)) {
        params.onClient();
        params.onEdgeServer();
        return;
    }
    params.onClient();
    params.onServer();
    return;
}
async function createEntrypoints(params) {
    const { config, pages, pagesDir, isDev, rootDir, rootPaths, appDir, appPaths, pageExtensions } = params;
    const edgeServer = {};
    const server = {};
    const client = {};
    let middlewareMatchers = undefined;
    let appPathsPerRoute = {};
    if (appDir && appPaths) {
        for(const pathname in appPaths){
            const normalizedPath = (0, _apppaths.normalizeAppPath)(pathname);
            const actualPath = appPaths[pathname];
            if (!appPathsPerRoute[normalizedPath]) {
                appPathsPerRoute[normalizedPath] = [];
            }
            appPathsPerRoute[normalizedPath].push(// TODO-APP: refactor to pass the page path from createPagesMapping instead.
            getPageFromPath(actualPath, pageExtensions).replace(_constants.APP_DIR_ALIAS, ''));
        }
        // TODO: find a better place to do this
        (0, _normalizecatchallroutes.normalizeCatchAllRoutes)(appPathsPerRoute);
        // Make sure to sort parallel routes to make the result deterministic.
        appPathsPerRoute = Object.fromEntries(Object.entries(appPathsPerRoute).map(([k, v])=>[
                k,
                v.sort()
            ]));
    }
    const getEntryHandler = (mappings, pagesType)=>async (page)=>{
            const bundleFile = (0, _normalizepagepath.normalizePagePath)(page);
            const clientBundlePath = _path.posix.join(pagesType, bundleFile);
            const serverBundlePath = pagesType === _pagetypes.PAGE_TYPES.PAGES ? _path.posix.join('pages', bundleFile) : pagesType === _pagetypes.PAGE_TYPES.APP ? _path.posix.join('app', bundleFile) : bundleFile.slice(1);
            const absolutePagePath = mappings[page];
            // Handle paths that have aliases
            const pageFilePath = getPageFilePath({
                absolutePagePath,
                pagesDir,
                appDir,
                rootDir
            });
            const isInsideAppDir = !!appDir && (absolutePagePath.startsWith(_constants.APP_DIR_ALIAS) || absolutePagePath.startsWith(appDir));
            const staticInfo = await getStaticInfoIncludingLayouts({
                isInsideAppDir,
                pageExtensions,
                pageFilePath,
                appDir,
                config,
                isDev,
                page
            });
            // TODO(timneutkens): remove this
            const isServerComponent = isInsideAppDir && staticInfo.rsc !== _constants1.RSC_MODULE_TYPES.client;
            if ((0, _utils.isMiddlewareFile)(page)) {
                var _staticInfo_middleware;
                middlewareMatchers = ((_staticInfo_middleware = staticInfo.middleware) == null ? void 0 : _staticInfo_middleware.matchers) ?? [
                    {
                        regexp: '.*',
                        originalSource: '/:path*'
                    }
                ];
            }
            const isInstrumentation = (0, _utils.isInstrumentationHookFile)(page) && pagesType === _pagetypes.PAGE_TYPES.ROOT;
            runDependingOnPageType({
                page,
                pageRuntime: staticInfo.runtime,
                pageType: pagesType,
                onClient: ()=>{
                    if (isServerComponent || isInsideAppDir) {
                    // We skip the initial entries for server component pages and let the
                    // server compiler inject them instead.
                    } else {
                        client[clientBundlePath] = getClientEntry({
                            absolutePagePath,
                            page
                        });
                    }
                },
                onServer: ()=>{
                    if (pagesType === 'app' && appDir) {
                        const matchedAppPaths = appPathsPerRoute[(0, _apppaths.normalizeAppPath)(page)];
                        server[serverBundlePath] = getAppEntry({
                            page,
                            name: serverBundlePath,
                            pagePath: absolutePagePath,
                            appDir,
                            appPaths: matchedAppPaths,
                            pageExtensions,
                            basePath: config.basePath,
                            assetPrefix: config.assetPrefix,
                            nextConfigOutput: config.output,
                            nextConfigExperimentalUseEarlyImport: config.experimental.useEarlyImport ? true : undefined,
                            preferredRegion: staticInfo.preferredRegion,
                            middlewareConfig: (0, _utils1.encodeToBase64)(staticInfo.middleware || {})
                        });
                    } else if (isInstrumentation) {
                        server[serverBundlePath.replace('src/', '')] = getInstrumentationEntry({
                            absolutePagePath,
                            isEdgeServer: false,
                            isDev: false
                        });
                    } else if ((0, _isapiroute.isAPIRoute)(page)) {
                        server[serverBundlePath] = [
                            (0, _nextrouteloader.getRouteLoaderEntry)({
                                kind: _routekind.RouteKind.PAGES_API,
                                page,
                                absolutePagePath,
                                preferredRegion: staticInfo.preferredRegion,
                                middlewareConfig: staticInfo.middleware || {}
                            })
                        ];
                    } else if (!(0, _utils.isMiddlewareFile)(page) && !(0, _isinternalcomponent.isInternalComponent)(absolutePagePath) && !(0, _isinternalcomponent.isNonRoutePagesPage)(page)) {
                        server[serverBundlePath] = [
                            (0, _nextrouteloader.getRouteLoaderEntry)({
                                kind: _routekind.RouteKind.PAGES,
                                page,
                                pages,
                                absolutePagePath,
                                preferredRegion: staticInfo.preferredRegion,
                                middlewareConfig: staticInfo.middleware ?? {}
                            })
                        ];
                    } else {
                        server[serverBundlePath] = [
                            absolutePagePath
                        ];
                    }
                },
                onEdgeServer: ()=>{
                    let appDirLoader = '';
                    if (isInstrumentation) {
                        edgeServer[serverBundlePath.replace('src/', '')] = getInstrumentationEntry({
                            absolutePagePath,
                            isEdgeServer: true,
                            isDev: false
                        });
                    } else {
                        if (pagesType === 'app') {
                            const matchedAppPaths = appPathsPerRoute[(0, _apppaths.normalizeAppPath)(page)];
                            appDirLoader = getAppEntry({
                                name: serverBundlePath,
                                page,
                                pagePath: absolutePagePath,
                                appDir: appDir,
                                appPaths: matchedAppPaths,
                                pageExtensions,
                                basePath: config.basePath,
                                assetPrefix: config.assetPrefix,
                                nextConfigOutput: config.output,
                                // This isn't used with edge as it needs to be set on the entry module, which will be the `edgeServerEntry` instead.
                                // Still passing it here for consistency.
                                preferredRegion: staticInfo.preferredRegion,
                                middlewareConfig: Buffer.from(JSON.stringify(staticInfo.middleware || {})).toString('base64')
                            }).import;
                        }
                        edgeServer[serverBundlePath] = getEdgeServerEntry({
                            ...params,
                            rootDir,
                            absolutePagePath: absolutePagePath,
                            bundlePath: clientBundlePath,
                            isDev: false,
                            isServerComponent,
                            page,
                            middleware: staticInfo == null ? void 0 : staticInfo.middleware,
                            pagesType,
                            appDirLoader,
                            preferredRegion: staticInfo.preferredRegion,
                            middlewareConfig: staticInfo.middleware
                        });
                    }
                }
            });
        };
    const promises = [];
    if (appPaths) {
        const entryHandler = getEntryHandler(appPaths, _pagetypes.PAGE_TYPES.APP);
        promises.push(Promise.all(Object.keys(appPaths).map(entryHandler)));
    }
    if (rootPaths) {
        promises.push(Promise.all(Object.keys(rootPaths).map(getEntryHandler(rootPaths, _pagetypes.PAGE_TYPES.ROOT))));
    }
    promises.push(Promise.all(Object.keys(pages).map(getEntryHandler(pages, _pagetypes.PAGE_TYPES.PAGES))));
    await Promise.all(promises);
    // Optimization: If there's only one instrumentation hook in edge compiler, which means there's no edge server entry.
    // We remove the edge instrumentation entry from edge compiler as it can be pure server side.
    if (edgeServer.instrumentation && Object.keys(edgeServer).length === 1) {
        delete edgeServer.instrumentation;
    }
    return {
        client,
        server,
        edgeServer,
        middlewareMatchers
    };
}
function finalizeEntrypoint({ name, compilerType, value, isServerComponent, hasAppDir }) {
    const entry = typeof value !== 'object' || Array.isArray(value) ? {
        import: value
    } : value;
    const isApi = name.startsWith('pages/api/');
    const isInstrumentation = (0, _utils.isInstrumentationHookFilename)(name);
    switch(compilerType){
        case _constants1.COMPILER_NAMES.server:
            {
                const layer = isApi ? _constants.WEBPACK_LAYERS.api : isInstrumentation ? _constants.WEBPACK_LAYERS.instrument : isServerComponent ? _constants.WEBPACK_LAYERS.reactServerComponents : undefined;
                return {
                    publicPath: isApi ? '' : undefined,
                    runtime: isApi ? 'webpack-api-runtime' : 'webpack-runtime',
                    layer,
                    ...entry
                };
            }
        case _constants1.COMPILER_NAMES.edgeServer:
            {
                return {
                    layer: isApi ? _constants.WEBPACK_LAYERS.api : (0, _utils.isMiddlewareFilename)(name) || isInstrumentation ? _constants.WEBPACK_LAYERS.middleware : undefined,
                    library: {
                        name: [
                            '_ENTRIES',
                            `middleware_[name]`
                        ],
                        type: 'assign'
                    },
                    runtime: _constants1.EDGE_RUNTIME_WEBPACK,
                    asyncChunks: false,
                    ...entry
                };
            }
        case _constants1.COMPILER_NAMES.client:
            {
                const isAppLayer = hasAppDir && (name === _constants1.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP || name === _constants1.APP_CLIENT_INTERNALS || name.startsWith('app/'));
                if (// Client special cases
                name !== _constants1.CLIENT_STATIC_FILES_RUNTIME_POLYFILLS && name !== _constants1.CLIENT_STATIC_FILES_RUNTIME_MAIN && name !== _constants1.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP && name !== _constants1.CLIENT_STATIC_FILES_RUNTIME_AMP && name !== _constants1.CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH) {
                    if (isAppLayer) {
                        return {
                            dependOn: _constants1.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP,
                            layer: _constants.WEBPACK_LAYERS.appPagesBrowser,
                            ...entry
                        };
                    }
                    return {
                        dependOn: name.startsWith('pages/') && name !== 'pages/_app' ? 'pages/_app' : _constants1.CLIENT_STATIC_FILES_RUNTIME_MAIN,
                        ...entry
                    };
                }
                if (isAppLayer) {
                    return {
                        layer: _constants.WEBPACK_LAYERS.appPagesBrowser,
                        ...entry
                    };
                }
                return entry;
            }
        default:
            {
                // Should never happen.
                throw new Error('Invalid compiler type');
            }
    }
}

//# sourceMappingURL=entries.js.map