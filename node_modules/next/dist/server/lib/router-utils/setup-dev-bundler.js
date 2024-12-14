"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    propagateServerField: null,
    setupDevBundler: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    propagateServerField: function() {
        return propagateServerField;
    },
    setupDevBundler: function() {
        return setupDevBundler;
    }
});
const _getpagestaticinfo = require("../../../build/analysis/get-page-static-info");
const _swc = require("../../../build/swc");
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _promises = require("fs/promises");
const _url = /*#__PURE__*/ _interop_require_default(require("url"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _querystring = /*#__PURE__*/ _interop_require_default(require("querystring"));
const _watchpack = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/watchpack"));
const _env = require("@next/env");
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _filesystem = require("./filesystem");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../../build/output/log"));
const _hotreloaderwebpack = /*#__PURE__*/ _interop_require_default(require("../../dev/hot-reloader-webpack"));
const _shared = require("../../../trace/shared");
const _loadjsconfig = /*#__PURE__*/ _interop_require_default(require("../../../build/load-jsconfig"));
const _findpagefile = require("../find-page-file");
const _events = require("../../../telemetry/events");
const _defineenvplugin = require("../../../build/webpack/plugins/define-env-plugin");
const _utils = require("../../../shared/lib/router/utils");
const _entries = require("../../../build/entries");
const _verifytypescriptsetup = require("../../../lib/verify-typescript-setup");
const _verifypartytownsetup = require("../../../lib/verify-partytown-setup");
const _routeregex = require("../../../shared/lib/router/utils/route-regex");
const _apppaths = require("../../../shared/lib/router/utils/app-paths");
const _builddataroute = require("./build-data-route");
const _routematcher = require("../../../shared/lib/router/utils/route-matcher");
const _normalizepathsep = require("../../../shared/lib/page-path/normalize-path-sep");
const _createclientrouterfilter = require("../../../lib/create-client-router-filter");
const _absolutepathtopage = require("../../../shared/lib/page-path/absolute-path-to-page");
const _generateinterceptionroutesrewrites = require("../../../lib/generate-interception-routes-rewrites");
const _constants = require("../../../shared/lib/constants");
const _middlewareroutematcher = require("../../../shared/lib/router/utils/middleware-route-matcher");
const _utils1 = require("../../../build/utils");
const _shared1 = require("../../../build/webpack/plugins/next-types-plugin/shared");
const _hotreloadertypes = require("../../dev/hot-reloader-types");
const _pagetypes = require("../../../lib/page-types");
const _hotreloaderturbopack = require("../../dev/hot-reloader-turbopack");
const _encryptionutilsserver = require("../../app-render/encryption-utils-server");
const _turbopackutils = require("../../dev/turbopack-utils");
const _ismetadataroute = require("../../../lib/metadata/is-metadata-route");
const _getmetadataroute = require("../../../lib/metadata/get-metadata-route");
const _createenvdefinitions = require("../experimental/create-env-definitions");
const _jsconfigpathsplugin = require("../../../build/webpack/plugins/jsconfig-paths-plugin");
const _store = require("../../../build/output/store");
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
async function verifyTypeScript(opts) {
    let usingTypeScript = false;
    const verifyResult = await (0, _verifytypescriptsetup.verifyTypeScriptSetup)({
        dir: opts.dir,
        distDir: opts.nextConfig.distDir,
        intentDirs: [
            opts.pagesDir,
            opts.appDir
        ].filter(Boolean),
        typeCheckPreflight: false,
        tsconfigPath: opts.nextConfig.typescript.tsconfigPath,
        disableStaticImages: opts.nextConfig.images.disableStaticImages,
        hasAppDir: !!opts.appDir,
        hasPagesDir: !!opts.pagesDir
    });
    if (verifyResult.version) {
        usingTypeScript = true;
    }
    return usingTypeScript;
}
async function propagateServerField(opts, field, args) {
    var _opts_renderServer_instance, _opts_renderServer;
    await ((_opts_renderServer = opts.renderServer) == null ? void 0 : (_opts_renderServer_instance = _opts_renderServer.instance) == null ? void 0 : _opts_renderServer_instance.propagateServerField(opts.dir, field, args));
}
async function startWatcher(opts) {
    const { nextConfig, appDir, pagesDir, dir, resetFetch } = opts;
    const { useFileSystemPublicRoutes } = nextConfig;
    const usingTypeScript = await verifyTypeScript(opts);
    const distDir = _path.default.join(opts.dir, opts.nextConfig.distDir);
    // we ensure the types directory exists here
    if (usingTypeScript) {
        const distTypesDir = _path.default.join(distDir, 'types');
        if (!_fs.default.existsSync(distTypesDir)) {
            await (0, _promises.mkdir)(distTypesDir, {
                recursive: true
            });
        }
    }
    (0, _shared.setGlobal)('distDir', distDir);
    (0, _shared.setGlobal)('phase', _constants.PHASE_DEVELOPMENT_SERVER);
    const validFileMatcher = (0, _findpagefile.createValidFileMatcher)(nextConfig.pageExtensions, appDir);
    const serverFields = {};
    // Update logging state once based on next.config.js when initializing
    _store.store.setState({
        logging: nextConfig.logging !== false
    });
    const hotReloader = opts.turbo ? await (0, _hotreloaderturbopack.createHotReloaderTurbopack)(opts, serverFields, distDir, resetFetch) : new _hotreloaderwebpack.default(opts.dir, {
        appDir,
        pagesDir,
        distDir,
        config: opts.nextConfig,
        buildId: 'development',
        encryptionKey: await (0, _encryptionutilsserver.generateEncryptionKeyBase64)({
            isBuild: false,
            distDir
        }),
        telemetry: opts.telemetry,
        rewrites: opts.fsChecker.rewrites,
        previewProps: opts.fsChecker.prerenderManifest.preview,
        resetFetch
    });
    await hotReloader.start();
    if (opts.nextConfig.experimental.nextScriptWorkers) {
        await (0, _verifypartytownsetup.verifyPartytownSetup)(opts.dir, _path.default.join(distDir, _constants.CLIENT_STATIC_FILES_PATH));
    }
    opts.fsChecker.ensureCallback(async function ensure(item) {
        if (item.type === 'appFile' || item.type === 'pageFile') {
            await hotReloader.ensurePage({
                clientOnly: false,
                page: item.itemPath,
                isApp: item.type === 'appFile',
                definition: undefined
            });
        }
    });
    let resolved = false;
    let prevSortedRoutes = [];
    await new Promise(async (resolve, reject)=>{
        if (pagesDir) {
            // Watchpack doesn't emit an event for an empty directory
            _fs.default.readdir(pagesDir, (_, files)=>{
                if (files == null ? void 0 : files.length) {
                    return;
                }
                if (!resolved) {
                    resolve();
                    resolved = true;
                }
            });
        }
        const pages = pagesDir ? [
            pagesDir
        ] : [];
        const app = appDir ? [
            appDir
        ] : [];
        const directories = [
            ...pages,
            ...app
        ];
        const rootDir = pagesDir || appDir;
        const files = [
            ...(0, _utils1.getPossibleMiddlewareFilenames)(_path.default.join(rootDir, '..'), nextConfig.pageExtensions),
            ...(0, _utils1.getPossibleInstrumentationHookFilenames)(_path.default.join(rootDir, '..'), nextConfig.pageExtensions)
        ];
        let nestedMiddleware = [];
        const envFiles = [
            '.env.development.local',
            '.env.local',
            '.env.development',
            '.env'
        ].map((file)=>_path.default.join(dir, file));
        files.push(...envFiles);
        // tsconfig/jsconfig paths hot-reloading
        const tsconfigPaths = [
            _path.default.join(dir, 'tsconfig.json'),
            _path.default.join(dir, 'jsconfig.json')
        ];
        files.push(...tsconfigPaths);
        const wp = new _watchpack.default({
            ignored: (pathname)=>{
                return !files.some((file)=>file.startsWith(pathname)) && !directories.some((d)=>pathname.startsWith(d) || d.startsWith(pathname));
            }
        });
        const fileWatchTimes = new Map();
        let enabledTypeScript = usingTypeScript;
        let previousClientRouterFilters;
        let previousConflictingPagePaths = new Set();
        wp.on('aggregated', async ()=>{
            var _serverFields_middleware, _serverFields_middleware1;
            let middlewareMatchers;
            const routedPages = [];
            const knownFiles = wp.getTimeInfoEntries();
            const appPaths = {};
            const pageNameSet = new Set();
            const conflictingAppPagePaths = new Set();
            const appPageFilePaths = new Map();
            const pagesPageFilePaths = new Map();
            const pagesWithUnsupportedSegments = new Map();
            let envChange = false;
            let tsconfigChange = false;
            let conflictingPageChange = 0;
            let hasRootAppNotFound = false;
            const { appFiles, pageFiles } = opts.fsChecker;
            appFiles.clear();
            pageFiles.clear();
            _shared1.devPageFiles.clear();
            pagesWithUnsupportedSegments.clear();
            const sortedKnownFiles = [
                ...knownFiles.keys()
            ].sort((0, _entries.sortByPageExts)(nextConfig.pageExtensions));
            for (const fileName of sortedKnownFiles){
                if (!files.includes(fileName) && !directories.some((d)=>fileName.startsWith(d))) {
                    continue;
                }
                const meta = knownFiles.get(fileName);
                const watchTime = fileWatchTimes.get(fileName);
                // If the file is showing up for the first time or the meta.timestamp is changed since last time
                const watchTimeChange = watchTime === undefined || watchTime && watchTime !== (meta == null ? void 0 : meta.timestamp);
                fileWatchTimes.set(fileName, meta == null ? void 0 : meta.timestamp);
                if (envFiles.includes(fileName)) {
                    if (watchTimeChange) {
                        envChange = true;
                    }
                    continue;
                }
                if (tsconfigPaths.includes(fileName)) {
                    if (fileName.endsWith('tsconfig.json')) {
                        enabledTypeScript = true;
                    }
                    if (watchTimeChange) {
                        tsconfigChange = true;
                    }
                    continue;
                }
                if ((meta == null ? void 0 : meta.accuracy) === undefined || !validFileMatcher.isPageFile(fileName)) {
                    continue;
                }
                const isAppPath = Boolean(appDir && (0, _normalizepathsep.normalizePathSep)(fileName).startsWith((0, _normalizepathsep.normalizePathSep)(appDir) + '/'));
                const isPagePath = Boolean(pagesDir && (0, _normalizepathsep.normalizePathSep)(fileName).startsWith((0, _normalizepathsep.normalizePathSep)(pagesDir) + '/'));
                const rootFile = (0, _absolutepathtopage.absolutePathToPage)(fileName, {
                    dir: dir,
                    extensions: nextConfig.pageExtensions,
                    keepIndex: false,
                    pagesType: _pagetypes.PAGE_TYPES.ROOT
                });
                if ((0, _utils1.isMiddlewareFile)(rootFile)) {
                    var _staticInfo_middleware;
                    const staticInfo = await (0, _entries.getStaticInfoIncludingLayouts)({
                        pageFilePath: fileName,
                        config: nextConfig,
                        appDir: appDir,
                        page: rootFile,
                        isDev: true,
                        isInsideAppDir: isAppPath,
                        pageExtensions: nextConfig.pageExtensions
                    });
                    if (nextConfig.output === 'export') {
                        _log.error('Middleware cannot be used with "output: export". See more info here: https://nextjs.org/docs/advanced-features/static-html-export');
                        continue;
                    }
                    serverFields.actualMiddlewareFile = rootFile;
                    await propagateServerField(opts, 'actualMiddlewareFile', serverFields.actualMiddlewareFile);
                    middlewareMatchers = ((_staticInfo_middleware = staticInfo.middleware) == null ? void 0 : _staticInfo_middleware.matchers) || [
                        {
                            regexp: '.*',
                            originalSource: '/:path*'
                        }
                    ];
                    continue;
                }
                if ((0, _utils1.isInstrumentationHookFile)(rootFile)) {
                    serverFields.actualInstrumentationHookFile = rootFile;
                    await propagateServerField(opts, 'actualInstrumentationHookFile', serverFields.actualInstrumentationHookFile);
                    continue;
                }
                if (fileName.endsWith('.ts') || fileName.endsWith('.tsx')) {
                    enabledTypeScript = true;
                }
                if (!(isAppPath || isPagePath)) {
                    continue;
                }
                // Collect all current filenames for the TS plugin to use
                _shared1.devPageFiles.add(fileName);
                let pageName = (0, _absolutepathtopage.absolutePathToPage)(fileName, {
                    dir: isAppPath ? appDir : pagesDir,
                    extensions: nextConfig.pageExtensions,
                    keepIndex: isAppPath,
                    pagesType: isAppPath ? _pagetypes.PAGE_TYPES.APP : _pagetypes.PAGE_TYPES.PAGES
                });
                if (isAppPath && appDir && (0, _ismetadataroute.isMetadataRouteFile)(fileName.replace(appDir, ''), nextConfig.pageExtensions, true)) {
                    const staticInfo = await (0, _getpagestaticinfo.getPageStaticInfo)({
                        pageFilePath: fileName,
                        nextConfig: {},
                        page: pageName,
                        isDev: true,
                        pageType: _pagetypes.PAGE_TYPES.APP
                    });
                    pageName = (0, _getmetadataroute.normalizeMetadataPageToRoute)(pageName, !!(staticInfo.generateSitemaps || staticInfo.generateImageMetadata));
                }
                if (!isAppPath && pageName.startsWith('/api/') && nextConfig.output === 'export') {
                    _log.error('API Routes cannot be used with "output: export". See more info here: https://nextjs.org/docs/advanced-features/static-html-export');
                    continue;
                }
                if (isAppPath) {
                    const isRootNotFound = validFileMatcher.isRootNotFound(fileName);
                    hasRootAppNotFound = true;
                    if (isRootNotFound) {
                        continue;
                    }
                    if (!isRootNotFound && !validFileMatcher.isAppRouterPage(fileName)) {
                        continue;
                    }
                    // Ignore files/directories starting with `_` in the app directory
                    if ((0, _normalizepathsep.normalizePathSep)(pageName).includes('/_')) {
                        continue;
                    }
                    const originalPageName = pageName;
                    pageName = (0, _apppaths.normalizeAppPath)(pageName).replace(/%5F/g, '_');
                    if (!appPaths[pageName]) {
                        appPaths[pageName] = [];
                    }
                    appPaths[pageName].push(originalPageName);
                    if (useFileSystemPublicRoutes) {
                        appFiles.add(pageName);
                    }
                    if (routedPages.includes(pageName)) {
                        continue;
                    }
                } else {
                    if (useFileSystemPublicRoutes) {
                        pageFiles.add(pageName);
                        // always add to nextDataRoutes for now but in future only add
                        // entries that actually use getStaticProps/getServerSideProps
                        opts.fsChecker.nextDataRoutes.add(pageName);
                    }
                }
                ;
                (isAppPath ? appPageFilePaths : pagesPageFilePaths).set(pageName, fileName);
                if (appDir && pageNameSet.has(pageName)) {
                    conflictingAppPagePaths.add(pageName);
                } else {
                    pageNameSet.add(pageName);
                }
                /**
         * If there is a middleware that is not declared in the root we will
         * warn without adding it so it doesn't make its way into the system.
         */ if (/[\\\\/]_middleware$/.test(pageName)) {
                    nestedMiddleware.push(pageName);
                    continue;
                }
                routedPages.push(pageName);
            }
            const numConflicting = conflictingAppPagePaths.size;
            conflictingPageChange = numConflicting - previousConflictingPagePaths.size;
            if (conflictingPageChange !== 0) {
                if (numConflicting > 0) {
                    let errorMessage = `Conflicting app and page file${numConflicting === 1 ? ' was' : 's were'} found, please remove the conflicting files to continue:\n`;
                    for (const p of conflictingAppPagePaths){
                        const appPath = _path.default.relative(dir, appPageFilePaths.get(p));
                        const pagesPath = _path.default.relative(dir, pagesPageFilePaths.get(p));
                        errorMessage += `  "${pagesPath}" - "${appPath}"\n`;
                    }
                    hotReloader.setHmrServerError(new Error(errorMessage));
                } else if (numConflicting === 0) {
                    hotReloader.clearHmrServerError();
                    await propagateServerField(opts, 'reloadMatchers', undefined);
                }
            }
            previousConflictingPagePaths = conflictingAppPagePaths;
            let clientRouterFilters;
            if (nextConfig.experimental.clientRouterFilter) {
                clientRouterFilters = (0, _createclientrouterfilter.createClientRouterFilter)(Object.keys(appPaths), nextConfig.experimental.clientRouterFilterRedirects ? (nextConfig._originalRedirects || []).filter((r)=>!r.internal) : [], nextConfig.experimental.clientRouterFilterAllowedRate);
                if (!previousClientRouterFilters || JSON.stringify(previousClientRouterFilters) !== JSON.stringify(clientRouterFilters)) {
                    envChange = true;
                    previousClientRouterFilters = clientRouterFilters;
                }
            }
            if (!usingTypeScript && enabledTypeScript) {
                // we tolerate the error here as this is best effort
                // and the manual install command will be shown
                await verifyTypeScript(opts).then(()=>{
                    tsconfigChange = true;
                }).catch(()=>{});
            }
            if (envChange || tsconfigChange) {
                var _hotReloader_activeWebpackConfigs;
                if (envChange) {
                    var _nextConfig_experimental;
                    const { loadedEnvFiles } = (0, _env.loadEnvConfig)(dir, process.env.NODE_ENV === 'development', _log, true, (envFilePath)=>{
                        _log.info(`Reload env: ${envFilePath}`);
                    });
                    if (usingTypeScript && ((_nextConfig_experimental = nextConfig.experimental) == null ? void 0 : _nextConfig_experimental.typedEnv)) {
                        // do not await, this is not essential for further process
                        (0, _createenvdefinitions.createEnvDefinitions)({
                            distDir,
                            loadedEnvFiles: [
                                ...loadedEnvFiles,
                                {
                                    path: nextConfig.configFileName,
                                    env: nextConfig.env,
                                    contents: ''
                                }
                            ]
                        });
                    }
                    await propagateServerField(opts, 'loadEnvConfig', [
                        {
                            dev: true,
                            forceReload: true,
                            silent: true
                        }
                    ]);
                }
                let tsconfigResult;
                if (tsconfigChange) {
                    try {
                        tsconfigResult = await (0, _loadjsconfig.default)(dir, nextConfig);
                    } catch (_) {
                    /* do we want to log if there are syntax errors in tsconfig while editing? */ }
                }
                if (hotReloader.turbopackProject) {
                    const hasRewrites = opts.fsChecker.rewrites.afterFiles.length > 0 || opts.fsChecker.rewrites.beforeFiles.length > 0 || opts.fsChecker.rewrites.fallback.length > 0;
                    await hotReloader.turbopackProject.update({
                        defineEnv: (0, _swc.createDefineEnv)({
                            isTurbopack: true,
                            clientRouterFilters,
                            config: nextConfig,
                            dev: true,
                            distDir,
                            fetchCacheKeyPrefix: opts.nextConfig.experimental.fetchCacheKeyPrefix,
                            hasRewrites,
                            // TODO: Implement
                            middlewareMatchers: undefined
                        })
                    });
                }
                (_hotReloader_activeWebpackConfigs = hotReloader.activeWebpackConfigs) == null ? void 0 : _hotReloader_activeWebpackConfigs.forEach((config, idx)=>{
                    const isClient = idx === 0;
                    const isNodeServer = idx === 1;
                    const isEdgeServer = idx === 2;
                    const hasRewrites = opts.fsChecker.rewrites.afterFiles.length > 0 || opts.fsChecker.rewrites.beforeFiles.length > 0 || opts.fsChecker.rewrites.fallback.length > 0;
                    if (tsconfigChange) {
                        var _config_resolve_plugins, _config_resolve;
                        (_config_resolve = config.resolve) == null ? void 0 : (_config_resolve_plugins = _config_resolve.plugins) == null ? void 0 : _config_resolve_plugins.forEach((plugin)=>{
                            // look for the JsConfigPathsPlugin and update with
                            // the latest paths/baseUrl config
                            if (plugin instanceof _jsconfigpathsplugin.JsConfigPathsPlugin && tsconfigResult) {
                                var _config_resolve_modules, _config_resolve, _jsConfig_compilerOptions;
                                const { resolvedBaseUrl, jsConfig } = tsconfigResult;
                                const currentResolvedBaseUrl = plugin.resolvedBaseUrl;
                                const resolvedUrlIndex = (_config_resolve = config.resolve) == null ? void 0 : (_config_resolve_modules = _config_resolve.modules) == null ? void 0 : _config_resolve_modules.findIndex((item)=>item === (currentResolvedBaseUrl == null ? void 0 : currentResolvedBaseUrl.baseUrl));
                                if (resolvedBaseUrl) {
                                    if (resolvedBaseUrl.baseUrl !== (currentResolvedBaseUrl == null ? void 0 : currentResolvedBaseUrl.baseUrl)) {
                                        // remove old baseUrl and add new one
                                        if (resolvedUrlIndex && resolvedUrlIndex > -1) {
                                            var _config_resolve_modules1, _config_resolve1;
                                            (_config_resolve1 = config.resolve) == null ? void 0 : (_config_resolve_modules1 = _config_resolve1.modules) == null ? void 0 : _config_resolve_modules1.splice(resolvedUrlIndex, 1);
                                        }
                                        // If the resolvedBaseUrl is implicit we only remove the previous value.
                                        // Only add the baseUrl if it's explicitly set in tsconfig/jsconfig
                                        if (!resolvedBaseUrl.isImplicit) {
                                            var _config_resolve_modules2, _config_resolve2;
                                            (_config_resolve2 = config.resolve) == null ? void 0 : (_config_resolve_modules2 = _config_resolve2.modules) == null ? void 0 : _config_resolve_modules2.push(resolvedBaseUrl.baseUrl);
                                        }
                                    }
                                }
                                if ((jsConfig == null ? void 0 : (_jsConfig_compilerOptions = jsConfig.compilerOptions) == null ? void 0 : _jsConfig_compilerOptions.paths) && resolvedBaseUrl) {
                                    Object.keys(plugin.paths).forEach((key)=>{
                                        delete plugin.paths[key];
                                    });
                                    Object.assign(plugin.paths, jsConfig.compilerOptions.paths);
                                    plugin.resolvedBaseUrl = resolvedBaseUrl;
                                }
                            }
                        });
                    }
                    if (envChange) {
                        var _config_plugins;
                        (_config_plugins = config.plugins) == null ? void 0 : _config_plugins.forEach((plugin)=>{
                            // we look for the DefinePlugin definitions so we can
                            // update them on the active compilers
                            if (plugin && typeof plugin.definitions === 'object' && plugin.definitions.__NEXT_DEFINE_ENV) {
                                const newDefine = (0, _defineenvplugin.getDefineEnv)({
                                    isTurbopack: false,
                                    clientRouterFilters,
                                    config: nextConfig,
                                    dev: true,
                                    distDir,
                                    fetchCacheKeyPrefix: opts.nextConfig.experimental.fetchCacheKeyPrefix,
                                    hasRewrites,
                                    isClient,
                                    isEdgeServer,
                                    isNodeOrEdgeCompilation: isNodeServer || isEdgeServer,
                                    isNodeServer,
                                    middlewareMatchers: undefined
                                });
                                Object.keys(plugin.definitions).forEach((key)=>{
                                    if (!(key in newDefine)) {
                                        delete plugin.definitions[key];
                                    }
                                });
                                Object.assign(plugin.definitions, newDefine);
                            }
                        });
                    }
                });
                await hotReloader.invalidate({
                    reloadAfterInvalidation: envChange
                });
            }
            if (nestedMiddleware.length > 0) {
                _log.error(new _utils1.NestedMiddlewareError(nestedMiddleware, dir, pagesDir || appDir).message);
                nestedMiddleware = [];
            }
            // Make sure to sort parallel routes to make the result deterministic.
            serverFields.appPathRoutes = Object.fromEntries(Object.entries(appPaths).map(([k, v])=>[
                    k,
                    v.sort()
                ]));
            await propagateServerField(opts, 'appPathRoutes', serverFields.appPathRoutes);
            // TODO: pass this to fsChecker/next-dev-server?
            serverFields.middleware = middlewareMatchers ? {
                match: null,
                page: '/',
                matchers: middlewareMatchers
            } : undefined;
            await propagateServerField(opts, 'middleware', serverFields.middleware);
            serverFields.hasAppNotFound = hasRootAppNotFound;
            opts.fsChecker.middlewareMatcher = ((_serverFields_middleware = serverFields.middleware) == null ? void 0 : _serverFields_middleware.matchers) ? (0, _middlewareroutematcher.getMiddlewareRouteMatcher)((_serverFields_middleware1 = serverFields.middleware) == null ? void 0 : _serverFields_middleware1.matchers) : undefined;
            const interceptionRoutes = (0, _generateinterceptionroutesrewrites.generateInterceptionRoutesRewrites)(Object.keys(appPaths), opts.nextConfig.basePath).map((item)=>(0, _filesystem.buildCustomRoute)('before_files_rewrite', item, opts.nextConfig.basePath, opts.nextConfig.experimental.caseSensitiveRoutes));
            opts.fsChecker.rewrites.beforeFiles.push(...interceptionRoutes);
            const exportPathMap = typeof nextConfig.exportPathMap === 'function' && await (nextConfig.exportPathMap == null ? void 0 : nextConfig.exportPathMap.call(nextConfig, {}, {
                dev: true,
                dir: opts.dir,
                outDir: null,
                distDir: distDir,
                buildId: 'development'
            })) || {};
            const exportPathMapEntries = Object.entries(exportPathMap || {});
            if (exportPathMapEntries.length > 0) {
                opts.fsChecker.exportPathMapRoutes = exportPathMapEntries.map(([key, value])=>(0, _filesystem.buildCustomRoute)('before_files_rewrite', {
                        source: key,
                        destination: `${value.page}${value.query ? '?' : ''}${_querystring.default.stringify(value.query)}`
                    }, opts.nextConfig.basePath, opts.nextConfig.experimental.caseSensitiveRoutes));
            }
            try {
                // we serve a separate manifest with all pages for the client in
                // dev mode so that we can match a page after a rewrite on the client
                // before it has been built and is populated in the _buildManifest
                const sortedRoutes = (0, _utils.getSortedRoutes)(routedPages);
                opts.fsChecker.dynamicRoutes = sortedRoutes.map((page)=>{
                    const regex = (0, _routeregex.getRouteRegex)(page);
                    return {
                        regex: regex.re.toString(),
                        match: (0, _routematcher.getRouteMatcher)(regex),
                        page
                    };
                });
                const dataRoutes = [];
                for (const page of sortedRoutes){
                    const route = (0, _builddataroute.buildDataRoute)(page, 'development');
                    const routeRegex = (0, _routeregex.getRouteRegex)(route.page);
                    dataRoutes.push({
                        ...route,
                        regex: routeRegex.re.toString(),
                        match: (0, _routematcher.getRouteMatcher)({
                            // TODO: fix this in the manifest itself, must also be fixed in
                            // upstream builder that relies on this
                            re: opts.nextConfig.i18n ? new RegExp(route.dataRouteRegex.replace(`/development/`, `/development/(?<nextLocale>[^/]+?)/`)) : new RegExp(route.dataRouteRegex),
                            groups: routeRegex.groups
                        })
                    });
                }
                opts.fsChecker.dynamicRoutes.unshift(...dataRoutes);
                if (!(prevSortedRoutes == null ? void 0 : prevSortedRoutes.every((val, idx)=>val === sortedRoutes[idx]))) {
                    const addedRoutes = sortedRoutes.filter((route)=>!prevSortedRoutes.includes(route));
                    const removedRoutes = prevSortedRoutes.filter((route)=>!sortedRoutes.includes(route));
                    // emit the change so clients fetch the update
                    hotReloader.send({
                        action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.DEV_PAGES_MANIFEST_UPDATE,
                        data: [
                            {
                                devPagesManifest: true
                            }
                        ]
                    });
                    addedRoutes.forEach((route)=>{
                        hotReloader.send({
                            action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.ADDED_PAGE,
                            data: [
                                route
                            ]
                        });
                    });
                    removedRoutes.forEach((route)=>{
                        hotReloader.send({
                            action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.REMOVED_PAGE,
                            data: [
                                route
                            ]
                        });
                    });
                }
                prevSortedRoutes = sortedRoutes;
                if (!resolved) {
                    resolve();
                    resolved = true;
                }
            } catch (e) {
                if (!resolved) {
                    reject(e);
                    resolved = true;
                } else {
                    _log.warn('Failed to reload dynamic routes:', e);
                }
            } finally{
                // Reload the matchers. The filesystem would have been written to,
                // and the matchers need to re-scan it to update the router.
                await propagateServerField(opts, 'reloadMatchers', undefined);
            }
        });
        wp.watch({
            directories: [
                dir
            ],
            startTime: 0
        });
    });
    const clientPagesManifestPath = `/_next/${_constants.CLIENT_STATIC_FILES_PATH}/development/${_constants.DEV_CLIENT_PAGES_MANIFEST}`;
    opts.fsChecker.devVirtualFsItems.add(clientPagesManifestPath);
    const devMiddlewareManifestPath = `/_next/${_constants.CLIENT_STATIC_FILES_PATH}/development/${_constants.DEV_CLIENT_MIDDLEWARE_MANIFEST}`;
    opts.fsChecker.devVirtualFsItems.add(devMiddlewareManifestPath);
    async function requestHandler(req, res) {
        var _parsedUrl_pathname, _parsedUrl_pathname1;
        const parsedUrl = _url.default.parse(req.url || '/');
        if ((_parsedUrl_pathname = parsedUrl.pathname) == null ? void 0 : _parsedUrl_pathname.includes(clientPagesManifestPath)) {
            res.statusCode = 200;
            res.setHeader('Content-Type', 'application/json; charset=utf-8');
            res.end(JSON.stringify({
                pages: prevSortedRoutes.filter((route)=>!opts.fsChecker.appFiles.has(route))
            }));
            return {
                finished: true
            };
        }
        if ((_parsedUrl_pathname1 = parsedUrl.pathname) == null ? void 0 : _parsedUrl_pathname1.includes(devMiddlewareManifestPath)) {
            var _serverFields_middleware;
            res.statusCode = 200;
            res.setHeader('Content-Type', 'application/json; charset=utf-8');
            res.end(JSON.stringify(((_serverFields_middleware = serverFields.middleware) == null ? void 0 : _serverFields_middleware.matchers) || []));
            return {
                finished: true
            };
        }
        return {
            finished: false
        };
    }
    function logErrorWithOriginalStack(err, type) {
        if (err instanceof _turbopackutils.ModuleBuildError) {
            // Errors that may come from issues from the user's code
            _log.error(err.message);
        } else if (err instanceof _turbopackutils.TurbopackInternalError) {
        // An internal Turbopack error that has been handled by next-swc, written
        // to disk and a simplified message shown to user on the Rust side.
        } else if (type === 'warning') {
            _log.warn(err);
        } else if (type === 'app-dir') {
            _log.error(err);
        } else if (type) {
            _log.error(`${type}:`, err);
        } else {
            _log.error(err);
        }
    }
    return {
        serverFields,
        hotReloader,
        requestHandler,
        logErrorWithOriginalStack,
        async ensureMiddleware (requestUrl) {
            if (!serverFields.actualMiddlewareFile) return;
            return hotReloader.ensurePage({
                page: serverFields.actualMiddlewareFile,
                clientOnly: false,
                definition: undefined,
                url: requestUrl
            });
        }
    };
}
async function setupDevBundler(opts) {
    const isSrcDir = _path.default.relative(opts.dir, opts.pagesDir || opts.appDir || '').startsWith('src');
    const result = await startWatcher(opts);
    opts.telemetry.record((0, _events.eventCliSession)(_path.default.join(opts.dir, opts.nextConfig.distDir), opts.nextConfig, {
        webpackVersion: 5,
        isSrcDir,
        turboFlag: !!opts.turbo,
        cliCommand: 'dev',
        appDir: !!opts.appDir,
        pagesDir: !!opts.pagesDir,
        isCustomServer: !!opts.isCustomServer,
        hasNowJson: !!await (0, _findup.default)('now.json', {
            cwd: opts.dir
        })
    }));
    return result;
}
 // Returns a trace rewritten through Turbopack's sourcemaps

//# sourceMappingURL=setup-dev-bundler.js.map