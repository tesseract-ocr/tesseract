"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    buildCustomRoute: null,
    setupFsCheck: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    buildCustomRoute: function() {
        return buildCustomRoute;
    },
    setupFsCheck: function() {
        return setupFsCheck;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../../build/output/log"));
const _debug = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/debug"));
const _lrucache = require("../lru-cache");
const _loadcustomroutes = /*#__PURE__*/ _interop_require_default(require("../../../lib/load-custom-routes"));
const _redirectstatus = require("../../../lib/redirect-status");
const _fileexists = require("../../../lib/file-exists");
const _recursivereaddir = require("../../../lib/recursive-readdir");
const _utils = require("../../../shared/lib/router/utils");
const _escaperegexp = require("../../../shared/lib/escape-regexp");
const _pathmatch = require("../../../shared/lib/router/utils/path-match");
const _routeregex = require("../../../shared/lib/router/utils/route-regex");
const _routematcher = require("../../../shared/lib/router/utils/route-matcher");
const _pathhasprefix = require("../../../shared/lib/router/utils/path-has-prefix");
const _normalizelocalepath = require("../../../shared/lib/i18n/normalize-locale-path");
const _removepathprefix = require("../../../shared/lib/router/utils/remove-path-prefix");
const _middlewareroutematcher = require("../../../shared/lib/router/utils/middleware-route-matcher");
const _constants = require("../../../shared/lib/constants");
const _normalizepathsep = require("../../../shared/lib/page-path/normalize-path-sep");
const _getmetadataroute = require("../../../lib/metadata/get-metadata-route");
const _rsc = require("../../normalizers/request/rsc");
const _prefetchrsc = require("../../normalizers/request/prefetch-rsc");
const _encodeuripath = require("../../../shared/lib/encode-uri-path");
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
const debug = (0, _debug.default)('next:router-server:filesystem');
const buildCustomRoute = (type, item, basePath, caseSensitive)=>{
    const restrictedRedirectPaths = [
        '/_next'
    ].map((p)=>basePath ? `${basePath}${p}` : p);
    const match = (0, _pathmatch.getPathMatch)(item.source, {
        strict: true,
        removeUnnamedParams: true,
        regexModifier: !item.internal ? (regex)=>(0, _redirectstatus.modifyRouteRegex)(regex, type === 'redirect' ? restrictedRedirectPaths : undefined) : undefined,
        sensitive: caseSensitive
    });
    return {
        ...item,
        ...type === 'rewrite' ? {
            check: true
        } : {},
        match
    };
};
async function setupFsCheck(opts) {
    const getItemsLru = !opts.dev ? new _lrucache.LRUCache(1024 * 1024, function length(value) {
        if (!value) return 0;
        return (value.fsPath || '').length + value.itemPath.length + value.type.length;
    }) : undefined;
    // routes that have _next/data endpoints (SSG/SSP)
    const nextDataRoutes = new Set();
    const publicFolderItems = new Set();
    const nextStaticFolderItems = new Set();
    const legacyStaticFolderItems = new Set();
    const appFiles = new Set();
    const pageFiles = new Set();
    let dynamicRoutes = [];
    let middlewareMatcher = ()=>false;
    const distDir = _path.default.join(opts.dir, opts.config.distDir);
    const publicFolderPath = _path.default.join(opts.dir, 'public');
    const nextStaticFolderPath = _path.default.join(distDir, 'static');
    const legacyStaticFolderPath = _path.default.join(opts.dir, 'static');
    let customRoutes = {
        redirects: [],
        rewrites: {
            beforeFiles: [],
            afterFiles: [],
            fallback: []
        },
        headers: []
    };
    let buildId = 'development';
    let prerenderManifest;
    if (!opts.dev) {
        var _middlewareManifest_middleware_, _middlewareManifest_middleware;
        const buildIdPath = _path.default.join(opts.dir, opts.config.distDir, _constants.BUILD_ID_FILE);
        try {
            buildId = await _promises.default.readFile(buildIdPath, 'utf8');
        } catch (err) {
            if (err.code !== 'ENOENT') throw err;
            throw new Error(`Could not find a production build in the '${opts.config.distDir}' directory. Try building your app with 'next build' before starting the production server. https://nextjs.org/docs/messages/production-start-no-build-id`);
        }
        try {
            for (const file of (await (0, _recursivereaddir.recursiveReadDir)(publicFolderPath))){
                // Ensure filename is encoded and normalized.
                publicFolderItems.add((0, _encodeuripath.encodeURIPath)((0, _normalizepathsep.normalizePathSep)(file)));
            }
        } catch (err) {
            if (err.code !== 'ENOENT') {
                throw err;
            }
        }
        try {
            for (const file of (await (0, _recursivereaddir.recursiveReadDir)(legacyStaticFolderPath))){
                // Ensure filename is encoded and normalized.
                legacyStaticFolderItems.add((0, _encodeuripath.encodeURIPath)((0, _normalizepathsep.normalizePathSep)(file)));
            }
            _log.warn(`The static directory has been deprecated in favor of the public directory. https://nextjs.org/docs/messages/static-dir-deprecated`);
        } catch (err) {
            if (err.code !== 'ENOENT') {
                throw err;
            }
        }
        try {
            for (const file of (await (0, _recursivereaddir.recursiveReadDir)(nextStaticFolderPath))){
                // Ensure filename is encoded and normalized.
                nextStaticFolderItems.add(_path.default.posix.join('/_next/static', (0, _encodeuripath.encodeURIPath)((0, _normalizepathsep.normalizePathSep)(file))));
            }
        } catch (err) {
            if (opts.config.output !== 'standalone') throw err;
        }
        const routesManifestPath = _path.default.join(distDir, _constants.ROUTES_MANIFEST);
        const prerenderManifestPath = _path.default.join(distDir, _constants.PRERENDER_MANIFEST);
        const middlewareManifestPath = _path.default.join(distDir, 'server', _constants.MIDDLEWARE_MANIFEST);
        const pagesManifestPath = _path.default.join(distDir, 'server', _constants.PAGES_MANIFEST);
        const appRoutesManifestPath = _path.default.join(distDir, _constants.APP_PATH_ROUTES_MANIFEST);
        const routesManifest = JSON.parse(await _promises.default.readFile(routesManifestPath, 'utf8'));
        prerenderManifest = JSON.parse(await _promises.default.readFile(prerenderManifestPath, 'utf8'));
        const middlewareManifest = JSON.parse(await _promises.default.readFile(middlewareManifestPath, 'utf8').catch(()=>'{}'));
        const pagesManifest = JSON.parse(await _promises.default.readFile(pagesManifestPath, 'utf8'));
        const appRoutesManifest = JSON.parse(await _promises.default.readFile(appRoutesManifestPath, 'utf8').catch(()=>'{}'));
        for (const key of Object.keys(pagesManifest)){
            // ensure the non-locale version is in the set
            if (opts.config.i18n) {
                pageFiles.add((0, _normalizelocalepath.normalizeLocalePath)(key, opts.config.i18n.locales).pathname);
            } else {
                pageFiles.add(key);
            }
        }
        for (const key of Object.keys(appRoutesManifest)){
            appFiles.add(appRoutesManifest[key]);
        }
        const escapedBuildId = (0, _escaperegexp.escapeStringRegexp)(buildId);
        for (const route of routesManifest.dataRoutes){
            if ((0, _utils.isDynamicRoute)(route.page)) {
                const routeRegex = (0, _routeregex.getRouteRegex)(route.page);
                dynamicRoutes.push({
                    ...route,
                    regex: routeRegex.re.toString(),
                    match: (0, _routematcher.getRouteMatcher)({
                        // TODO: fix this in the manifest itself, must also be fixed in
                        // upstream builder that relies on this
                        re: opts.config.i18n ? new RegExp(route.dataRouteRegex.replace(`/${escapedBuildId}/`, `/${escapedBuildId}/(?<nextLocale>[^/]+?)/`)) : new RegExp(route.dataRouteRegex),
                        groups: routeRegex.groups
                    })
                });
            }
            nextDataRoutes.add(route.page);
        }
        for (const route of routesManifest.dynamicRoutes){
            dynamicRoutes.push({
                ...route,
                match: (0, _routematcher.getRouteMatcher)((0, _routeregex.getRouteRegex)(route.page))
            });
        }
        if ((_middlewareManifest_middleware = middlewareManifest.middleware) == null ? void 0 : (_middlewareManifest_middleware_ = _middlewareManifest_middleware['/']) == null ? void 0 : _middlewareManifest_middleware_.matchers) {
            var _middlewareManifest_middleware_1, _middlewareManifest_middleware1;
            middlewareMatcher = (0, _middlewareroutematcher.getMiddlewareRouteMatcher)((_middlewareManifest_middleware1 = middlewareManifest.middleware) == null ? void 0 : (_middlewareManifest_middleware_1 = _middlewareManifest_middleware1['/']) == null ? void 0 : _middlewareManifest_middleware_1.matchers);
        }
        customRoutes = {
            redirects: routesManifest.redirects,
            rewrites: routesManifest.rewrites ? Array.isArray(routesManifest.rewrites) ? {
                beforeFiles: [],
                afterFiles: routesManifest.rewrites,
                fallback: []
            } : routesManifest.rewrites : {
                beforeFiles: [],
                afterFiles: [],
                fallback: []
            },
            headers: routesManifest.headers
        };
    } else {
        // dev handling
        customRoutes = await (0, _loadcustomroutes.default)(opts.config);
        prerenderManifest = {
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
    }
    const headers = customRoutes.headers.map((item)=>buildCustomRoute('header', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes));
    const redirects = customRoutes.redirects.map((item)=>buildCustomRoute('redirect', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes));
    const rewrites = {
        beforeFiles: customRoutes.rewrites.beforeFiles.map((item)=>buildCustomRoute('before_files_rewrite', item)),
        afterFiles: customRoutes.rewrites.afterFiles.map((item)=>buildCustomRoute('rewrite', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes)),
        fallback: customRoutes.rewrites.fallback.map((item)=>buildCustomRoute('rewrite', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes))
    };
    const { i18n } = opts.config;
    const handleLocale = (pathname, locales)=>{
        let locale;
        if (i18n) {
            const i18nResult = (0, _normalizelocalepath.normalizeLocalePath)(pathname, locales || i18n.locales);
            pathname = i18nResult.pathname;
            locale = i18nResult.detectedLocale;
        }
        return {
            locale,
            pathname
        };
    };
    debug('nextDataRoutes', nextDataRoutes);
    debug('dynamicRoutes', dynamicRoutes);
    debug('pageFiles', pageFiles);
    debug('appFiles', appFiles);
    let ensureFn;
    const normalizers = {
        // Because we can't know if the app directory is enabled or not at this
        // stage, we assume that it is.
        rsc: new _rsc.RSCPathnameNormalizer(),
        prefetchRSC: opts.config.experimental.ppr ? new _prefetchrsc.PrefetchRSCPathnameNormalizer() : undefined
    };
    return {
        headers,
        rewrites,
        redirects,
        buildId,
        handleLocale,
        appFiles,
        pageFiles,
        dynamicRoutes,
        nextDataRoutes,
        exportPathMapRoutes: undefined,
        devVirtualFsItems: new Set(),
        prerenderManifest,
        middlewareMatcher: middlewareMatcher,
        ensureCallback (fn) {
            ensureFn = fn;
        },
        async getItem (itemPath) {
            const originalItemPath = itemPath;
            const itemKey = originalItemPath;
            const lruResult = getItemsLru == null ? void 0 : getItemsLru.get(itemKey);
            if (lruResult) {
                return lruResult;
            }
            const { basePath } = opts.config;
            const hasBasePath = (0, _pathhasprefix.pathHasPrefix)(itemPath, basePath);
            // Return null if path doesn't start with basePath
            if (basePath && !hasBasePath) {
                return null;
            }
            // Remove basePath if it exists.
            if (basePath && hasBasePath) {
                itemPath = (0, _removepathprefix.removePathPrefix)(itemPath, basePath) || '/';
            }
            // Simulate minimal mode requests by normalizing RSC and postponed
            // requests.
            if (opts.minimalMode) {
                var _normalizers_prefetchRSC;
                if ((_normalizers_prefetchRSC = normalizers.prefetchRSC) == null ? void 0 : _normalizers_prefetchRSC.match(itemPath)) {
                    itemPath = normalizers.prefetchRSC.normalize(itemPath, true);
                } else if (normalizers.rsc.match(itemPath)) {
                    itemPath = normalizers.rsc.normalize(itemPath, true);
                }
            }
            if (itemPath !== '/' && itemPath.endsWith('/')) {
                itemPath = itemPath.substring(0, itemPath.length - 1);
            }
            let decodedItemPath = itemPath;
            try {
                decodedItemPath = decodeURIComponent(itemPath);
            } catch  {}
            if (itemPath === '/_next/image') {
                return {
                    itemPath,
                    type: 'nextImage'
                };
            }
            const itemsToCheck = [
                [
                    this.devVirtualFsItems,
                    'devVirtualFsItem'
                ],
                [
                    nextStaticFolderItems,
                    'nextStaticFolder'
                ],
                [
                    legacyStaticFolderItems,
                    'legacyStaticFolder'
                ],
                [
                    publicFolderItems,
                    'publicFolder'
                ],
                [
                    appFiles,
                    'appFile'
                ],
                [
                    pageFiles,
                    'pageFile'
                ]
            ];
            for (let [items, type] of itemsToCheck){
                let locale;
                let curItemPath = itemPath;
                let curDecodedItemPath = decodedItemPath;
                const isDynamicOutput = type === 'pageFile' || type === 'appFile';
                if (i18n) {
                    var _i18n_domains;
                    const localeResult = handleLocale(itemPath, // legacy behavior allows visiting static assets under
                    // default locale but no other locale
                    isDynamicOutput ? undefined : [
                        i18n == null ? void 0 : i18n.defaultLocale,
                        // default locales from domains need to be matched too
                        ...((_i18n_domains = i18n.domains) == null ? void 0 : _i18n_domains.map((item)=>item.defaultLocale)) || []
                    ]);
                    if (localeResult.pathname !== curItemPath) {
                        curItemPath = localeResult.pathname;
                        locale = localeResult.locale;
                        try {
                            curDecodedItemPath = decodeURIComponent(curItemPath);
                        } catch  {}
                    }
                }
                if (type === 'legacyStaticFolder') {
                    if (!(0, _pathhasprefix.pathHasPrefix)(curItemPath, '/static')) {
                        continue;
                    }
                    curItemPath = curItemPath.substring('/static'.length);
                    try {
                        curDecodedItemPath = decodeURIComponent(curItemPath);
                    } catch  {}
                }
                if (type === 'nextStaticFolder' && !(0, _pathhasprefix.pathHasPrefix)(curItemPath, '/_next/static')) {
                    continue;
                }
                const nextDataPrefix = `/_next/data/${buildId}/`;
                if (type === 'pageFile' && curItemPath.startsWith(nextDataPrefix) && curItemPath.endsWith('.json')) {
                    items = nextDataRoutes;
                    // remove _next/data/<build-id> prefix
                    curItemPath = curItemPath.substring(nextDataPrefix.length - 1);
                    // remove .json postfix
                    curItemPath = curItemPath.substring(0, curItemPath.length - '.json'.length);
                    const curLocaleResult = handleLocale(curItemPath);
                    curItemPath = curLocaleResult.pathname === '/index' ? '/' : curLocaleResult.pathname;
                    locale = curLocaleResult.locale;
                    try {
                        curDecodedItemPath = decodeURIComponent(curItemPath);
                    } catch  {}
                }
                let matchedItem = items.has(curItemPath);
                // check decoded variant as well
                if (!matchedItem && !opts.dev) {
                    matchedItem = items.has(curDecodedItemPath);
                    if (matchedItem) curItemPath = curDecodedItemPath;
                    else {
                        // x-ref: https://github.com/vercel/next.js/issues/54008
                        // There're cases that urls get decoded before requests, we should support both encoded and decoded ones.
                        // e.g. nginx could decode the proxy urls, the below ones should be treated as the same:
                        // decoded version: `/_next/static/chunks/pages/blog/[slug]-d4858831b91b69f6.js`
                        // encoded version: `/_next/static/chunks/pages/blog/%5Bslug%5D-d4858831b91b69f6.js`
                        try {
                            // encode the special characters in the path and retrieve again to determine if path exists.
                            const encodedCurItemPath = (0, _encodeuripath.encodeURIPath)(curItemPath);
                            matchedItem = items.has(encodedCurItemPath);
                        } catch  {}
                    }
                }
                if (matchedItem || opts.dev) {
                    let fsPath;
                    let itemsRoot;
                    switch(type){
                        case 'nextStaticFolder':
                            {
                                itemsRoot = nextStaticFolderPath;
                                curItemPath = curItemPath.substring('/_next/static'.length);
                                break;
                            }
                        case 'legacyStaticFolder':
                            {
                                itemsRoot = legacyStaticFolderPath;
                                break;
                            }
                        case 'publicFolder':
                            {
                                itemsRoot = publicFolderPath;
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    if (itemsRoot && curItemPath) {
                        fsPath = _path.default.posix.join(itemsRoot, curItemPath);
                    }
                    // dynamically check fs in development so we don't
                    // have to wait on the watcher
                    if (!matchedItem && opts.dev) {
                        const isStaticAsset = [
                            'nextStaticFolder',
                            'publicFolder',
                            'legacyStaticFolder'
                        ].includes(type);
                        if (isStaticAsset && itemsRoot) {
                            let found = fsPath && await (0, _fileexists.fileExists)(fsPath, _fileexists.FileType.File);
                            if (!found) {
                                try {
                                    // In dev, we ensure encoded paths match
                                    // decoded paths on the filesystem so check
                                    // that variation as well
                                    const tempItemPath = decodeURIComponent(curItemPath);
                                    fsPath = _path.default.posix.join(itemsRoot, tempItemPath);
                                    found = await (0, _fileexists.fileExists)(fsPath, _fileexists.FileType.File);
                                } catch  {}
                                if (!found) {
                                    continue;
                                }
                            }
                        } else if (type === 'pageFile' || type === 'appFile') {
                            var _ensureFn;
                            const isAppFile = type === 'appFile';
                            if (ensureFn && await ((_ensureFn = ensureFn({
                                type,
                                itemPath: isAppFile ? (0, _getmetadataroute.normalizeMetadataRoute)(curItemPath) : curItemPath
                            })) == null ? void 0 : _ensureFn.catch(()=>'ENSURE_FAILED')) === 'ENSURE_FAILED') {
                                continue;
                            }
                        } else {
                            continue;
                        }
                    }
                    // i18n locales aren't matched for app dir
                    if (type === 'appFile' && locale && locale !== (i18n == null ? void 0 : i18n.defaultLocale)) {
                        continue;
                    }
                    const itemResult = {
                        type,
                        fsPath,
                        locale,
                        itemsRoot,
                        itemPath: curItemPath
                    };
                    getItemsLru == null ? void 0 : getItemsLru.set(itemKey, itemResult);
                    return itemResult;
                }
            }
            getItemsLru == null ? void 0 : getItemsLru.set(itemKey, null);
            return null;
        },
        getDynamicRoutes () {
            // this should include data routes
            return this.dynamicRoutes;
        },
        getMiddlewareMatchers () {
            return this.middlewareMatcher;
        }
    };
}

//# sourceMappingURL=filesystem.js.map