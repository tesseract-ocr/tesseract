"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    NestedMiddlewareError: null,
    buildAppStaticPaths: null,
    buildStaticPaths: null,
    collectMeta: null,
    collectRoutesUsingEdgeRuntime: null,
    computeFromManifest: null,
    copyTracedFiles: null,
    detectConflictingPaths: null,
    difference: null,
    getDefinedNamedExports: null,
    getJsPageSizeInKb: null,
    getPossibleInstrumentationHookFilenames: null,
    getPossibleMiddlewareFilenames: null,
    getSupportedBrowsers: null,
    hasCustomGetInitialProps: null,
    isAppBuiltinNotFoundPage: null,
    isCustomErrorPage: null,
    isInstrumentationHookFile: null,
    isInstrumentationHookFilename: null,
    isMiddlewareFile: null,
    isMiddlewareFilename: null,
    isPageStatic: null,
    isReservedPage: null,
    isWebpackAppPagesLayer: null,
    isWebpackBundledLayer: null,
    isWebpackClientOnlyLayer: null,
    isWebpackDefaultLayer: null,
    isWebpackServerOnlyLayer: null,
    printCustomRoutes: null,
    printTreeView: null,
    reduceAppConfig: null,
    unique: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    NestedMiddlewareError: function() {
        return NestedMiddlewareError;
    },
    buildAppStaticPaths: function() {
        return buildAppStaticPaths;
    },
    buildStaticPaths: function() {
        return buildStaticPaths;
    },
    collectMeta: function() {
        return collectMeta;
    },
    collectRoutesUsingEdgeRuntime: function() {
        return collectRoutesUsingEdgeRuntime;
    },
    computeFromManifest: function() {
        return computeFromManifest;
    },
    copyTracedFiles: function() {
        return copyTracedFiles;
    },
    detectConflictingPaths: function() {
        return detectConflictingPaths;
    },
    difference: function() {
        return difference;
    },
    getDefinedNamedExports: function() {
        return getDefinedNamedExports;
    },
    getJsPageSizeInKb: function() {
        return getJsPageSizeInKb;
    },
    getPossibleInstrumentationHookFilenames: function() {
        return getPossibleInstrumentationHookFilenames;
    },
    getPossibleMiddlewareFilenames: function() {
        return getPossibleMiddlewareFilenames;
    },
    getSupportedBrowsers: function() {
        return getSupportedBrowsers;
    },
    hasCustomGetInitialProps: function() {
        return hasCustomGetInitialProps;
    },
    isAppBuiltinNotFoundPage: function() {
        return isAppBuiltinNotFoundPage;
    },
    isCustomErrorPage: function() {
        return isCustomErrorPage;
    },
    isInstrumentationHookFile: function() {
        return isInstrumentationHookFile;
    },
    isInstrumentationHookFilename: function() {
        return isInstrumentationHookFilename;
    },
    isMiddlewareFile: function() {
        return isMiddlewareFile;
    },
    isMiddlewareFilename: function() {
        return isMiddlewareFilename;
    },
    isPageStatic: function() {
        return isPageStatic;
    },
    isReservedPage: function() {
        return isReservedPage;
    },
    isWebpackAppPagesLayer: function() {
        return isWebpackAppPagesLayer;
    },
    isWebpackBundledLayer: function() {
        return isWebpackBundledLayer;
    },
    isWebpackClientOnlyLayer: function() {
        return isWebpackClientOnlyLayer;
    },
    isWebpackDefaultLayer: function() {
        return isWebpackDefaultLayer;
    },
    isWebpackServerOnlyLayer: function() {
        return isWebpackServerOnlyLayer;
    },
    printCustomRoutes: function() {
        return printCustomRoutes;
    },
    printTreeView: function() {
        return printTreeView;
    },
    reduceAppConfig: function() {
        return reduceAppConfig;
    },
    unique: function() {
        return unique;
    }
});
require("../server/require-hook");
require("../server/node-polyfill-crypto");
require("../server/node-environment");
const _picocolors = require("../lib/picocolors");
const _gzipsize = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/gzip-size"));
const _texttable = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/text-table"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _fs = require("fs");
const _reactis = require("next/dist/compiled/react-is");
const _stripansi = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/strip-ansi"));
const _browserslist = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/browserslist"));
const _constants = require("../lib/constants");
const _constants1 = require("../shared/lib/constants");
const _prettybytes = /*#__PURE__*/ _interop_require_default(require("../lib/pretty-bytes"));
const _routeregex = require("../shared/lib/router/utils/route-regex");
const _routematcher = require("../shared/lib/router/utils/route-matcher");
const _isdynamic = require("../shared/lib/router/utils/is-dynamic");
const _escapepathdelimiters = /*#__PURE__*/ _interop_require_default(require("../shared/lib/router/utils/escape-path-delimiters"));
const _findpagefile = require("../server/lib/find-page-file");
const _removetrailingslash = require("../shared/lib/router/utils/remove-trailing-slash");
const _isedgeruntime = require("../lib/is-edge-runtime");
const _normalizelocalepath = require("../shared/lib/i18n/normalize-locale-path");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("./output/log"));
const _loadcomponents = require("../server/load-components");
const _trace = require("../trace");
const _setuphttpagentenv = require("../server/setup-http-agent-env");
const _asyncsema = require("next/dist/compiled/async-sema");
const _denormalizepagepath = require("../shared/lib/page-path/denormalize-page-path");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _sandbox = require("../server/web/sandbox");
const _clientreference = require("../lib/client-reference");
const _workstore = require("../server/async-storage/work-store");
const _incrementalcache = require("../server/lib/incremental-cache");
const _nodefsmethods = require("../server/lib/node-fs-methods");
const _ciinfo = /*#__PURE__*/ _interop_require_wildcard(require("../server/ci-info"));
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _denormalizeapppath = require("../shared/lib/page-path/denormalize-app-path");
const _routekind = require("../server/route-kind");
const _interopdefault = require("../lib/interop-default");
const _formatdynamicimportpath = require("../lib/format-dynamic-import-path");
const _interceptionroutes = require("../server/lib/interception-routes");
const _ppr = require("../server/lib/experimental/ppr");
const _fallback = require("../lib/fallback");
const _fallbackparams = require("../server/request/fallback-params");
const _appsegments = require("./segment-config/app/app-segments");
const _createincrementalcache = require("../export/helpers/create-incremental-cache");
const _runwithafter = require("../server/after/run-with-after");
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
// Use `print()` for expected console output
const print = console.log;
const RESERVED_PAGE = /^\/(_app|_error|_document|api(\/|$))/;
const fileGzipStats = {};
const fsStatGzip = (file)=>{
    const cached = fileGzipStats[file];
    if (cached) return cached;
    return fileGzipStats[file] = _gzipsize.default.file(file);
};
const fileSize = async (file)=>(await _fs.promises.stat(file)).size;
const fileStats = {};
const fsStat = (file)=>{
    const cached = fileStats[file];
    if (cached) return cached;
    return fileStats[file] = fileSize(file);
};
function unique(main, sub) {
    return [
        ...new Set([
            ...main,
            ...sub
        ])
    ];
}
function difference(main, sub) {
    const a = new Set(main);
    const b = new Set(sub);
    return [
        ...a
    ].filter((x)=>!b.has(x));
}
/**
 * Return an array of the items shared by both arrays.
 */ function intersect(main, sub) {
    const a = new Set(main);
    const b = new Set(sub);
    return [
        ...new Set([
            ...a
        ].filter((x)=>b.has(x)))
    ];
}
function sum(a) {
    return a.reduce((size, stat)=>size + stat, 0);
}
let cachedBuildManifest;
let cachedAppBuildManifest;
let lastCompute;
let lastComputePageInfo;
async function computeFromManifest(manifests, distPath, gzipSize = true, pageInfos) {
    var _manifests_app, _files_app;
    if (Object.is(cachedBuildManifest, manifests.build) && lastComputePageInfo === !!pageInfos && Object.is(cachedAppBuildManifest, manifests.app)) {
        return lastCompute;
    }
    // Determine the files that are in pages and app and count them, this will
    // tell us if they are unique or common.
    const countBuildFiles = (map, key, manifest)=>{
        for (const file of manifest[key]){
            if (key === '/_app') {
                map.set(file, Infinity);
            } else if (map.has(file)) {
                map.set(file, map.get(file) + 1);
            } else {
                map.set(file, 1);
            }
        }
    };
    const files = {
        pages: {
            each: new Map(),
            expected: 0
        }
    };
    for(const key in manifests.build.pages){
        if (pageInfos) {
            const pageInfo = pageInfos.get(key);
            // don't include AMP pages since they don't rely on shared bundles
            // AMP First pages are not under the pageInfos key
            if (pageInfo == null ? void 0 : pageInfo.isHybridAmp) {
                continue;
            }
        }
        files.pages.expected++;
        countBuildFiles(files.pages.each, key, manifests.build.pages);
    }
    // Collect the build files form the app manifest.
    if ((_manifests_app = manifests.app) == null ? void 0 : _manifests_app.pages) {
        files.app = {
            each: new Map(),
            expected: 0
        };
        for(const key in manifests.app.pages){
            files.app.expected++;
            countBuildFiles(files.app.each, key, manifests.app.pages);
        }
    }
    const getSize = gzipSize ? fsStatGzip : fsStat;
    const stats = new Map();
    // For all of the files in the pages and app manifests, compute the file size
    // at once.
    await Promise.all([
        ...new Set([
            ...files.pages.each.keys(),
            ...((_files_app = files.app) == null ? void 0 : _files_app.each.keys()) ?? []
        ])
    ].map(async (f)=>{
        try {
            // Add the file size to the stats.
            stats.set(f, await getSize(_path.default.join(distPath, f)));
        } catch  {}
    }));
    const groupFiles = async (listing)=>{
        const entries = [
            ...listing.each.entries()
        ];
        const shapeGroup = (group)=>group.reduce((acc, [f])=>{
                acc.files.push(f);
                const size = stats.get(f);
                if (typeof size === 'number') {
                    acc.size.total += size;
                }
                return acc;
            }, {
                files: [],
                size: {
                    total: 0
                }
            });
        return {
            unique: shapeGroup(entries.filter(([, len])=>len === 1)),
            common: shapeGroup(entries.filter(([, len])=>len === listing.expected || len === Infinity))
        };
    };
    lastCompute = {
        router: {
            pages: await groupFiles(files.pages),
            app: files.app ? await groupFiles(files.app) : undefined
        },
        sizes: stats
    };
    cachedBuildManifest = manifests.build;
    cachedAppBuildManifest = manifests.app;
    lastComputePageInfo = !!pageInfos;
    return lastCompute;
}
function isMiddlewareFilename(file) {
    return file === _constants.MIDDLEWARE_FILENAME || file === `src/${_constants.MIDDLEWARE_FILENAME}`;
}
function isInstrumentationHookFilename(file) {
    return file === _constants.INSTRUMENTATION_HOOK_FILENAME || file === `src/${_constants.INSTRUMENTATION_HOOK_FILENAME}`;
}
const filterAndSortList = (list, routeType, hasCustomApp)=>{
    let pages;
    if (routeType === 'app') {
        // filter out static app route of /favicon.ico
        pages = list.filter((e)=>e !== '/favicon.ico');
    } else {
        // filter built-in pages
        pages = list.slice().filter((e)=>!(e === '/_document' || e === '/_error' || !hasCustomApp && e === '/_app'));
    }
    return pages.sort((a, b)=>a.localeCompare(b));
};
function collectRoutesUsingEdgeRuntime(input) {
    const routesUsingEdgeRuntime = {};
    for (const [route, info] of input.entries()){
        if ((0, _isedgeruntime.isEdgeRuntime)(info.runtime)) {
            routesUsingEdgeRuntime[route] = 0;
        }
    }
    return routesUsingEdgeRuntime;
}
async function printTreeView(lists, pageInfos, { distPath, buildId, pagesDir, pageExtensions, buildManifest, appBuildManifest, middlewareManifest, useStaticPages404, gzipSize = true }) {
    var _lists_app, _middlewareManifest_middleware;
    const getPrettySize = (_size)=>{
        const size = (0, _prettybytes.default)(_size);
        return (0, _picocolors.white)((0, _picocolors.bold)(size));
    };
    const MIN_DURATION = 300;
    const getPrettyDuration = (_duration)=>{
        const duration = `${_duration} ms`;
        // green for 300-1000ms
        if (_duration < 1000) return (0, _picocolors.green)(duration);
        // yellow for 1000-2000ms
        if (_duration < 2000) return (0, _picocolors.yellow)(duration);
        // red for >= 2000ms
        return (0, _picocolors.red)((0, _picocolors.bold)(duration));
    };
    const getCleanName = (fileName)=>fileName// Trim off `static/`
        .replace(/^static\//, '')// Re-add `static/` for root files
        .replace(/^<buildId>/, 'static')// Remove file hash
        .replace(/(?:^|[.-])([0-9a-z]{6})[0-9a-z]{14}(?=\.)/, '.$1');
    // Check if we have a custom app.
    const hasCustomApp = !!(pagesDir && await (0, _findpagefile.findPageFile)(pagesDir, '/_app', pageExtensions, false));
    // Collect all the symbols we use so we can print the icons out.
    const usedSymbols = new Set();
    const messages = [];
    const stats = await computeFromManifest({
        build: buildManifest,
        app: appBuildManifest
    }, distPath, gzipSize, pageInfos);
    const printFileTree = async ({ list, routerType })=>{
        var _stats_router_routerType, _stats_router_routerType1;
        const filteredPages = filterAndSortList(list, routerType, hasCustomApp);
        if (filteredPages.length === 0) {
            return;
        }
        messages.push([
            routerType === 'app' ? 'Route (app)' : 'Route (pages)',
            'Size',
            'First Load JS'
        ].map((entry)=>(0, _picocolors.underline)(entry)));
        filteredPages.forEach((item, i, arr)=>{
            var _pageInfo_ssgPageDurations, _buildManifest_pages_item, _pageInfo_ssgPageRoutes;
            const border = i === 0 ? arr.length === 1 ? '─' : '┌' : i === arr.length - 1 ? '└' : '├';
            const pageInfo = pageInfos.get(item);
            const ampFirst = buildManifest.ampFirstPages.includes(item);
            const totalDuration = ((pageInfo == null ? void 0 : pageInfo.pageDuration) || 0) + ((pageInfo == null ? void 0 : (_pageInfo_ssgPageDurations = pageInfo.ssgPageDurations) == null ? void 0 : _pageInfo_ssgPageDurations.reduce((a, b)=>a + (b || 0), 0)) || 0);
            let symbol;
            if (item === '/_app' || item === '/_app.server') {
                symbol = ' ';
            } else if ((0, _isedgeruntime.isEdgeRuntime)(pageInfo == null ? void 0 : pageInfo.runtime)) {
                symbol = 'ƒ';
            } else if (pageInfo == null ? void 0 : pageInfo.isRoutePPREnabled) {
                if (// If the page has an empty prelude, then it's equivalent to a dynamic page
                (pageInfo == null ? void 0 : pageInfo.hasEmptyPrelude) || // ensure we don't mark dynamic paths that postponed as being dynamic
                // since in this case we're able to partially prerender it
                pageInfo.isDynamicAppRoute && !pageInfo.hasPostponed) {
                    symbol = 'ƒ';
                } else if (!(pageInfo == null ? void 0 : pageInfo.hasPostponed)) {
                    symbol = '○';
                } else {
                    symbol = '◐';
                }
            } else if (pageInfo == null ? void 0 : pageInfo.isStatic) {
                symbol = '○';
            } else if (pageInfo == null ? void 0 : pageInfo.isSSG) {
                symbol = '●';
            } else {
                symbol = 'ƒ';
            }
            usedSymbols.add(symbol);
            if (pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds) usedSymbols.add('ISR');
            messages.push([
                `${border} ${symbol} ${(pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds) ? `${item} (ISR: ${pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds} Seconds)` : item}${totalDuration > MIN_DURATION ? ` (${getPrettyDuration(totalDuration)})` : ''}`,
                pageInfo ? ampFirst ? (0, _picocolors.cyan)('AMP') : pageInfo.size >= 0 ? (0, _prettybytes.default)(pageInfo.size) : '' : '',
                pageInfo ? ampFirst ? (0, _picocolors.cyan)('AMP') : pageInfo.size >= 0 ? getPrettySize(pageInfo.totalSize) : '' : ''
            ]);
            const uniqueCssFiles = ((_buildManifest_pages_item = buildManifest.pages[item]) == null ? void 0 : _buildManifest_pages_item.filter((file)=>{
                var _stats_router_routerType;
                return file.endsWith('.css') && ((_stats_router_routerType = stats.router[routerType]) == null ? void 0 : _stats_router_routerType.unique.files.includes(file));
            })) || [];
            if (uniqueCssFiles.length > 0) {
                const contSymbol = i === arr.length - 1 ? ' ' : '├';
                uniqueCssFiles.forEach((file, index, { length })=>{
                    const innerSymbol = index === length - 1 ? '└' : '├';
                    const size = stats.sizes.get(file);
                    messages.push([
                        `${contSymbol}   ${innerSymbol} ${getCleanName(file)}`,
                        typeof size === 'number' ? (0, _prettybytes.default)(size) : '',
                        ''
                    ]);
                });
            }
            if (pageInfo == null ? void 0 : (_pageInfo_ssgPageRoutes = pageInfo.ssgPageRoutes) == null ? void 0 : _pageInfo_ssgPageRoutes.length) {
                const totalRoutes = pageInfo.ssgPageRoutes.length;
                const contSymbol = i === arr.length - 1 ? ' ' : '├';
                let routes;
                if (pageInfo.ssgPageDurations && pageInfo.ssgPageDurations.some((d)=>d > MIN_DURATION)) {
                    const previewPages = totalRoutes === 8 ? 8 : Math.min(totalRoutes, 7);
                    const routesWithDuration = pageInfo.ssgPageRoutes.map((route, idx)=>({
                            route,
                            duration: pageInfo.ssgPageDurations[idx] || 0
                        })).sort(({ duration: a }, { duration: b })=>// Sort by duration
                        // keep too small durations in original order at the end
                        a <= MIN_DURATION && b <= MIN_DURATION ? 0 : b - a);
                    routes = routesWithDuration.slice(0, previewPages);
                    const remainingRoutes = routesWithDuration.slice(previewPages);
                    if (remainingRoutes.length) {
                        const remaining = remainingRoutes.length;
                        const avgDuration = Math.round(remainingRoutes.reduce((total, { duration })=>total + duration, 0) / remainingRoutes.length);
                        routes.push({
                            route: `[+${remaining} more paths]`,
                            duration: 0,
                            avgDuration
                        });
                    }
                } else {
                    const previewPages = totalRoutes === 4 ? 4 : Math.min(totalRoutes, 3);
                    routes = pageInfo.ssgPageRoutes.slice(0, previewPages).map((route)=>({
                            route,
                            duration: 0
                        }));
                    if (totalRoutes > previewPages) {
                        const remaining = totalRoutes - previewPages;
                        routes.push({
                            route: `[+${remaining} more paths]`,
                            duration: 0
                        });
                    }
                }
                routes.forEach(({ route, duration, avgDuration }, index, { length })=>{
                    const innerSymbol = index === length - 1 ? '└' : '├';
                    messages.push([
                        `${contSymbol}   ${innerSymbol} ${route}${duration > MIN_DURATION ? ` (${getPrettyDuration(duration)})` : ''}${avgDuration && avgDuration > MIN_DURATION ? ` (avg ${getPrettyDuration(avgDuration)})` : ''}`,
                        '',
                        ''
                    ]);
                });
            }
        });
        const sharedFilesSize = (_stats_router_routerType = stats.router[routerType]) == null ? void 0 : _stats_router_routerType.common.size.total;
        const sharedFiles = ((_stats_router_routerType1 = stats.router[routerType]) == null ? void 0 : _stats_router_routerType1.common.files) ?? [];
        messages.push([
            '+ First Load JS shared by all',
            typeof sharedFilesSize === 'number' ? getPrettySize(sharedFilesSize) : '',
            ''
        ]);
        const sharedCssFiles = [];
        const sharedJsChunks = [
            ...sharedFiles.filter((file)=>{
                if (file.endsWith('.css')) {
                    sharedCssFiles.push(file);
                    return false;
                }
                return true;
            }).map((e)=>e.replace(buildId, '<buildId>')).sort(),
            ...sharedCssFiles.map((e)=>e.replace(buildId, '<buildId>')).sort()
        ];
        // if the some chunk are less than 10kb or we don't know the size, we only show the total size of the rest
        const tenKbLimit = 10 * 1000;
        let restChunkSize = 0;
        let restChunkCount = 0;
        sharedJsChunks.forEach((fileName, index, { length })=>{
            const innerSymbol = index + restChunkCount === length - 1 ? '└' : '├';
            const originalName = fileName.replace('<buildId>', buildId);
            const cleanName = getCleanName(fileName);
            const size = stats.sizes.get(originalName);
            if (!size || size < tenKbLimit) {
                restChunkCount++;
                restChunkSize += size || 0;
                return;
            }
            messages.push([
                `  ${innerSymbol} ${cleanName}`,
                (0, _prettybytes.default)(size),
                ''
            ]);
        });
        if (restChunkCount > 0) {
            messages.push([
                `  └ other shared chunks (total)`,
                (0, _prettybytes.default)(restChunkSize),
                ''
            ]);
        }
    };
    // If enabled, then print the tree for the app directory.
    if (lists.app && stats.router.app) {
        await printFileTree({
            routerType: 'app',
            list: lists.app
        });
        messages.push([
            '',
            '',
            ''
        ]);
    }
    pageInfos.set('/404', {
        ...pageInfos.get('/404') || pageInfos.get('/_error'),
        isStatic: useStaticPages404
    });
    // If there's no app /_notFound page present, then the 404 is still using the pages/404
    if (!lists.pages.includes('/404') && !((_lists_app = lists.app) == null ? void 0 : _lists_app.includes(_constants1.UNDERSCORE_NOT_FOUND_ROUTE))) {
        lists.pages = [
            ...lists.pages,
            '/404'
        ];
    }
    // Print the tree view for the pages directory.
    await printFileTree({
        routerType: 'pages',
        list: lists.pages
    });
    const middlewareInfo = (_middlewareManifest_middleware = middlewareManifest.middleware) == null ? void 0 : _middlewareManifest_middleware['/'];
    if ((middlewareInfo == null ? void 0 : middlewareInfo.files.length) > 0) {
        const middlewareSizes = await Promise.all(middlewareInfo.files.map((dep)=>`${distPath}/${dep}`).map(gzipSize ? fsStatGzip : fsStat));
        messages.push([
            '',
            '',
            ''
        ]);
        messages.push([
            'ƒ Middleware',
            getPrettySize(sum(middlewareSizes)),
            ''
        ]);
    }
    print((0, _texttable.default)(messages, {
        align: [
            'l',
            'l',
            'r'
        ],
        stringLength: (str)=>(0, _stripansi.default)(str).length
    }));
    const staticFunctionInfo = lists.app && stats.router.app ? 'generateStaticParams' : 'getStaticProps';
    print();
    print((0, _texttable.default)([
        usedSymbols.has('○') && [
            '○',
            '(Static)',
            'prerendered as static content'
        ],
        usedSymbols.has('●') && [
            '●',
            '(SSG)',
            `prerendered as static HTML (uses ${(0, _picocolors.cyan)(staticFunctionInfo)})`
        ],
        usedSymbols.has('ISR') && [
            '',
            '(ISR)',
            `incremental static regeneration (uses revalidate in ${(0, _picocolors.cyan)(staticFunctionInfo)})`
        ],
        usedSymbols.has('◐') && [
            '◐',
            '(Partial Prerender)',
            'prerendered as static HTML with dynamic server-streamed content'
        ],
        usedSymbols.has('ƒ') && [
            'ƒ',
            '(Dynamic)',
            `server-rendered on demand`
        ]
    ].filter((x)=>x), {
        align: [
            'l',
            'l',
            'l'
        ],
        stringLength: (str)=>(0, _stripansi.default)(str).length
    }));
    print();
}
function printCustomRoutes({ redirects, rewrites, headers }) {
    const printRoutes = (routes, type)=>{
        const isRedirects = type === 'Redirects';
        const isHeaders = type === 'Headers';
        print((0, _picocolors.underline)(type));
        /*
        ┌ source
        ├ permanent/statusCode
        └ destination
     */ const routesStr = routes.map((route)=>{
            let routeStr = `┌ source: ${route.source}\n`;
            if (!isHeaders) {
                const r = route;
                routeStr += `${isRedirects ? '├' : '└'} destination: ${r.destination}\n`;
            }
            if (isRedirects) {
                const r = route;
                routeStr += `└ ${r.statusCode ? `status: ${r.statusCode}` : `permanent: ${r.permanent}`}\n`;
            }
            if (isHeaders) {
                const r = route;
                routeStr += `└ headers:\n`;
                for(let i = 0; i < r.headers.length; i++){
                    const header = r.headers[i];
                    const last = i === headers.length - 1;
                    routeStr += `  ${last ? '└' : '├'} ${header.key}: ${header.value}\n`;
                }
            }
            return routeStr;
        }).join('\n');
        print(`${routesStr}\n`);
    };
    print();
    if (redirects.length) {
        printRoutes(redirects, 'Redirects');
    }
    if (headers.length) {
        printRoutes(headers, 'Headers');
    }
    const combinedRewrites = [
        ...rewrites.beforeFiles,
        ...rewrites.afterFiles,
        ...rewrites.fallback
    ];
    if (combinedRewrites.length) {
        printRoutes(combinedRewrites, 'Rewrites');
    }
}
async function getJsPageSizeInKb(routerType, page, distPath, buildManifest, appBuildManifest, gzipSize = true, cachedStats) {
    const pageManifest = routerType === 'pages' ? buildManifest : appBuildManifest;
    if (!pageManifest) {
        throw new Error('expected appBuildManifest with an "app" pageType');
    }
    // Normalize appBuildManifest keys
    if (routerType === 'app') {
        pageManifest.pages = Object.entries(pageManifest.pages).reduce((acc, [key, value])=>{
            const newKey = (0, _apppaths.normalizeAppPath)(key);
            acc[newKey] = value;
            return acc;
        }, {});
    }
    // If stats was not provided, then compute it again.
    const stats = cachedStats ?? await computeFromManifest({
        build: buildManifest,
        app: appBuildManifest
    }, distPath, gzipSize);
    const pageData = stats.router[routerType];
    if (!pageData) {
        // This error shouldn't happen and represents an error in Next.js.
        throw new Error('expected "app" manifest data with an "app" pageType');
    }
    const pagePath = routerType === 'pages' ? (0, _denormalizepagepath.denormalizePagePath)(page) : (0, _denormalizeapppath.denormalizeAppPagePath)(page);
    const fnFilterJs = (entry)=>entry.endsWith('.js');
    const pageFiles = (pageManifest.pages[pagePath] ?? []).filter(fnFilterJs);
    const appFiles = (pageManifest.pages['/_app'] ?? []).filter(fnFilterJs);
    const fnMapRealPath = (dep)=>`${distPath}/${dep}`;
    const allFilesReal = unique(pageFiles, appFiles).map(fnMapRealPath);
    const selfFilesReal = difference(// Find the files shared by the pages files and the unique files...
    intersect(pageFiles, pageData.unique.files), // but without the common files.
    pageData.common.files).map(fnMapRealPath);
    const getSize = gzipSize ? fsStatGzip : fsStat;
    // Try to get the file size from the page data if available, otherwise do a
    // raw compute.
    const getCachedSize = async (file)=>{
        const key = file.slice(distPath.length + 1);
        const size = stats.sizes.get(key);
        // If the size wasn't in the stats bundle, then get it from the file
        // directly.
        if (typeof size !== 'number') {
            return getSize(file);
        }
        return size;
    };
    try {
        // Doesn't use `Promise.all`, as we'd double compute duplicate files. This
        // function is memoized, so the second one will instantly resolve.
        const allFilesSize = sum(await Promise.all(allFilesReal.map(getCachedSize)));
        const selfFilesSize = sum(await Promise.all(selfFilesReal.map(getCachedSize)));
        return [
            selfFilesSize,
            allFilesSize
        ];
    } catch  {}
    return [
        -1,
        -1
    ];
}
async function buildStaticPaths({ page, getStaticPaths, staticPathsResult, configFileName, locales, defaultLocale, appDir }) {
    const prerenderedRoutes = [];
    const _routeRegex = (0, _routeregex.getRouteRegex)(page);
    const _routeMatcher = (0, _routematcher.getRouteMatcher)(_routeRegex);
    // Get the default list of allowed params.
    const routeParameterKeys = Object.keys(_routeMatcher(page));
    if (!staticPathsResult) {
        if (getStaticPaths) {
            staticPathsResult = await getStaticPaths({
                locales,
                defaultLocale
            });
        } else {
            throw new Error(`invariant: attempted to buildStaticPaths without "staticPathsResult" or "getStaticPaths" ${page}`);
        }
    }
    const expectedReturnVal = `Expected: { paths: [], fallback: boolean }\n` + `See here for more info: https://nextjs.org/docs/messages/invalid-getstaticpaths-value`;
    if (!staticPathsResult || typeof staticPathsResult !== 'object' || Array.isArray(staticPathsResult)) {
        throw new Error(`Invalid value returned from getStaticPaths in ${page}. Received ${typeof staticPathsResult} ${expectedReturnVal}`);
    }
    const invalidStaticPathKeys = Object.keys(staticPathsResult).filter((key)=>!(key === 'paths' || key === 'fallback'));
    if (invalidStaticPathKeys.length > 0) {
        throw new Error(`Extra keys returned from getStaticPaths in ${page} (${invalidStaticPathKeys.join(', ')}) ${expectedReturnVal}`);
    }
    if (!(typeof staticPathsResult.fallback === 'boolean' || staticPathsResult.fallback === 'blocking')) {
        throw new Error(`The \`fallback\` key must be returned from getStaticPaths in ${page}.\n` + expectedReturnVal);
    }
    const toPrerender = staticPathsResult.paths;
    if (!Array.isArray(toPrerender)) {
        throw new Error(`Invalid \`paths\` value returned from getStaticPaths in ${page}.\n` + `\`paths\` must be an array of strings or objects of shape { params: [key: string]: string }`);
    }
    toPrerender.forEach((entry)=>{
        // For a string-provided path, we must make sure it matches the dynamic
        // route.
        if (typeof entry === 'string') {
            entry = (0, _removetrailingslash.removeTrailingSlash)(entry);
            const localePathResult = (0, _normalizelocalepath.normalizeLocalePath)(entry, locales);
            let cleanedEntry = entry;
            if (localePathResult.detectedLocale) {
                cleanedEntry = entry.slice(localePathResult.detectedLocale.length + 1);
            } else if (defaultLocale) {
                entry = `/${defaultLocale}${entry}`;
            }
            const result = _routeMatcher(cleanedEntry);
            if (!result) {
                throw new Error(`The provided path \`${cleanedEntry}\` does not match the page: \`${page}\`.`);
            }
            // If leveraging the string paths variant the entry should already be
            // encoded so we decode the segments ensuring we only escape path
            // delimiters
            prerenderedRoutes.push({
                path: entry.split('/').map((segment)=>(0, _escapepathdelimiters.default)(decodeURIComponent(segment), true)).join('/'),
                encoded: entry,
                fallbackRouteParams: undefined
            });
        } else {
            const invalidKeys = Object.keys(entry).filter((key)=>key !== 'params' && key !== 'locale');
            if (invalidKeys.length) {
                throw new Error(`Additional keys were returned from \`getStaticPaths\` in page "${page}". ` + `URL Parameters intended for this dynamic route must be nested under the \`params\` key, i.e.:` + `\n\n\treturn { params: { ${routeParameterKeys.map((k)=>`${k}: ...`).join(', ')} } }` + `\n\nKeys that need to be moved: ${invalidKeys.join(', ')}.\n`);
            }
            const { params = {} } = entry;
            let builtPage = page;
            let encodedBuiltPage = page;
            routeParameterKeys.forEach((validParamKey)=>{
                const { repeat, optional } = _routeRegex.groups[validParamKey];
                let paramValue = params[validParamKey];
                if (optional && params.hasOwnProperty(validParamKey) && (paramValue === null || paramValue === undefined || paramValue === false)) {
                    paramValue = [];
                }
                if (repeat && !Array.isArray(paramValue) || !repeat && typeof paramValue !== 'string') {
                    // If this is from app directory, and not all params were provided,
                    // then filter this out.
                    if (appDir && typeof paramValue === 'undefined') {
                        builtPage = '';
                        encodedBuiltPage = '';
                        return;
                    }
                    throw new Error(`A required parameter (${validParamKey}) was not provided as ${repeat ? 'an array' : 'a string'} received ${typeof paramValue} in ${appDir ? 'generateStaticParams' : 'getStaticPaths'} for ${page}`);
                }
                let replaced = `[${repeat ? '...' : ''}${validParamKey}]`;
                if (optional) {
                    replaced = `[${replaced}]`;
                }
                builtPage = builtPage.replace(replaced, repeat ? paramValue.map((segment)=>(0, _escapepathdelimiters.default)(segment, true)).join('/') : (0, _escapepathdelimiters.default)(paramValue, true)).replace(/\\/g, '/').replace(/(?!^)\/$/, '');
                encodedBuiltPage = encodedBuiltPage.replace(replaced, repeat ? paramValue.map(encodeURIComponent).join('/') : encodeURIComponent(paramValue)).replace(/\\/g, '/').replace(/(?!^)\/$/, '');
            });
            if (!builtPage && !encodedBuiltPage) {
                return;
            }
            if (entry.locale && !(locales == null ? void 0 : locales.includes(entry.locale))) {
                throw new Error(`Invalid locale returned from getStaticPaths for ${page}, the locale ${entry.locale} is not specified in ${configFileName}`);
            }
            const curLocale = entry.locale || defaultLocale || '';
            prerenderedRoutes.push({
                path: `${curLocale ? `/${curLocale}` : ''}${curLocale && builtPage === '/' ? '' : builtPage}`,
                encoded: `${curLocale ? `/${curLocale}` : ''}${curLocale && encodedBuiltPage === '/' ? '' : encodedBuiltPage}`,
                fallbackRouteParams: undefined
            });
        }
    });
    const seen = new Set();
    return {
        fallbackMode: (0, _fallback.parseStaticPathsResult)(staticPathsResult.fallback),
        prerenderedRoutes: prerenderedRoutes.filter((route)=>{
            if (seen.has(route.path)) return false;
            // Filter out duplicate paths.
            seen.add(route.path);
            return true;
        })
    };
}
async function buildAppStaticPaths({ dir, page, distDir, dynamicIO, authInterrupts, configFileName, segments, isrFlushToDisk, cacheHandler, cacheLifeProfiles, requestHeaders, maxMemoryCacheSize, fetchCacheKeyPrefix, nextConfigOutput, ComponentMod, isRoutePPREnabled, buildId }) {
    if (segments.some((generate)=>{
        var _generate_config;
        return ((_generate_config = generate.config) == null ? void 0 : _generate_config.dynamicParams) === true;
    }) && nextConfigOutput === 'export') {
        throw new Error('"dynamicParams: true" cannot be used with "output: export". See more info here: https://nextjs.org/docs/app/building-your-application/deploying/static-exports');
    }
    ComponentMod.patchFetch();
    let CurCacheHandler;
    if (cacheHandler) {
        CurCacheHandler = (0, _interopdefault.interopDefault)(await import((0, _formatdynamicimportpath.formatDynamicImportPath)(dir, cacheHandler)).then((mod)=>mod.default || mod));
    }
    const incrementalCache = new _incrementalcache.IncrementalCache({
        fs: _nodefsmethods.nodeFs,
        dev: true,
        dynamicIO,
        flushToDisk: isrFlushToDisk,
        serverDistDir: _path.default.join(distDir, 'server'),
        fetchCacheKeyPrefix,
        maxMemoryCacheSize,
        getPrerenderManifest: ()=>({
                version: -1,
                routes: {},
                dynamicRoutes: {},
                notFoundRoutes: [],
                preview: null
            }),
        CurCacheHandler,
        requestHeaders,
        minimalMode: _ciinfo.hasNextSupport
    });
    const paramKeys = new Set();
    const staticParamKeys = new Set();
    for (const segment of segments){
        if (segment.param) {
            var _segment_config;
            paramKeys.add(segment.param);
            if (((_segment_config = segment.config) == null ? void 0 : _segment_config.dynamicParams) === false) {
                staticParamKeys.add(segment.param);
            }
        }
    }
    const afterRunner = new _runwithafter.AfterRunner();
    const store = (0, _workstore.createWorkStore)({
        page,
        // We're discovering the parameters here, so we don't have any unknown
        // ones.
        fallbackRouteParams: null,
        renderOpts: {
            incrementalCache,
            cacheLifeProfiles,
            supportsDynamicResponse: true,
            isRevalidate: false,
            experimental: {
                dynamicIO,
                authInterrupts
            },
            waitUntil: afterRunner.context.waitUntil,
            onClose: afterRunner.context.onClose,
            onAfterTaskError: afterRunner.context.onTaskError,
            buildId
        }
    });
    const routeParams = await ComponentMod.workAsyncStorage.run(store, async ()=>{
        async function builtRouteParams(parentsParams = [], idx = 0) {
            // If we don't have any more to process, then we're done.
            if (idx === segments.length) return parentsParams;
            const current = segments[idx];
            if (typeof current.generateStaticParams !== 'function' && idx < segments.length) {
                return builtRouteParams(parentsParams, idx + 1);
            }
            const params = [];
            if (current.generateStaticParams) {
                var _current_config;
                // fetchCache can be used to inform the fetch() defaults used inside
                // of generateStaticParams. revalidate and dynamic options don't come into
                // play within generateStaticParams.
                if (typeof ((_current_config = current.config) == null ? void 0 : _current_config.fetchCache) !== 'undefined') {
                    store.fetchCache = current.config.fetchCache;
                }
                if (parentsParams.length > 0) {
                    for (const parentParams of parentsParams){
                        const result = await current.generateStaticParams({
                            params: parentParams
                        });
                        for (const item of result){
                            params.push({
                                ...parentParams,
                                ...item
                            });
                        }
                    }
                } else {
                    const result = await current.generateStaticParams({
                        params: {}
                    });
                    params.push(...result);
                }
            }
            if (idx < segments.length) {
                return builtRouteParams(params, idx + 1);
            }
            return params;
        }
        return builtRouteParams();
    });
    let lastDynamicSegmentHadGenerateStaticParams = false;
    for (const segment of segments){
        var _segment_config1;
        // Check to see if there are any missing params for segments that have
        // dynamicParams set to false.
        if (segment.param && segment.isDynamicSegment && ((_segment_config1 = segment.config) == null ? void 0 : _segment_config1.dynamicParams) === false) {
            for (const params of routeParams){
                if (segment.param in params) continue;
                const relative = segment.filePath ? _path.default.relative(dir, segment.filePath) : undefined;
                throw new Error(`Segment "${relative}" exports "dynamicParams: false" but the param "${segment.param}" is missing from the generated route params.`);
            }
        }
        if (segment.isDynamicSegment && typeof segment.generateStaticParams !== 'function') {
            lastDynamicSegmentHadGenerateStaticParams = false;
        } else if (typeof segment.generateStaticParams === 'function') {
            lastDynamicSegmentHadGenerateStaticParams = true;
        }
    }
    // Determine if all the segments have had their parameters provided. If there
    // was no dynamic parameters, then we've collected all the params.
    const hadAllParamsGenerated = paramKeys.size === 0 || routeParams.length > 0 && routeParams.every((params)=>{
        for (const key of paramKeys){
            if (key in params) continue;
            return false;
        }
        return true;
    });
    // TODO: dynamic params should be allowed to be granular per segment but
    // we need additional information stored/leveraged in the prerender
    // manifest to allow this behavior.
    const dynamicParams = segments.every((segment)=>{
        var _segment_config;
        return ((_segment_config = segment.config) == null ? void 0 : _segment_config.dynamicParams) !== false;
    });
    const supportsRoutePreGeneration = hadAllParamsGenerated || process.env.NODE_ENV === 'production';
    const fallbackMode = dynamicParams ? supportsRoutePreGeneration ? isRoutePPREnabled ? _fallback.FallbackMode.PRERENDER : _fallback.FallbackMode.BLOCKING_STATIC_RENDER : undefined : _fallback.FallbackMode.NOT_FOUND;
    let result = {
        fallbackMode,
        prerenderedRoutes: lastDynamicSegmentHadGenerateStaticParams ? [] : undefined
    };
    if (hadAllParamsGenerated && fallbackMode) {
        result = await buildStaticPaths({
            staticPathsResult: {
                fallback: (0, _fallback.fallbackModeToStaticPathsResult)(fallbackMode),
                paths: routeParams.map((params)=>({
                        params
                    }))
            },
            page,
            configFileName,
            appDir: true
        });
    }
    // If the fallback mode is a prerender, we want to include the dynamic
    // route in the prerendered routes too.
    if (isRoutePPREnabled) {
        result.prerenderedRoutes ??= [];
        result.prerenderedRoutes.unshift({
            path: page,
            encoded: page,
            fallbackRouteParams: (0, _fallbackparams.getParamKeys)(page)
        });
    }
    await afterRunner.executeAfter();
    return result;
}
async function isPageStatic({ dir, page, distDir, configFileName, runtimeEnvConfig, httpAgentOptions, locales, defaultLocale, parentId, pageRuntime, edgeInfo, pageType, dynamicIO, authInterrupts, originalAppPath, isrFlushToDisk, maxMemoryCacheSize, nextConfigOutput, cacheHandler, cacheHandlers, cacheLifeProfiles, pprConfig, buildId }) {
    await (0, _createincrementalcache.createIncrementalCache)({
        cacheHandler,
        cacheHandlers,
        distDir,
        dir,
        dynamicIO,
        flushToDisk: isrFlushToDisk,
        cacheMaxMemorySize: maxMemoryCacheSize
    });
    const isPageStaticSpan = (0, _trace.trace)('is-page-static-utils', parentId);
    return isPageStaticSpan.traceAsyncFn(async ()=>{
        require('../shared/lib/runtime-config.external').setConfig(runtimeEnvConfig);
        (0, _setuphttpagentenv.setHttpClientAndAgentOptions)({
            httpAgentOptions
        });
        let componentsResult;
        let prerenderedRoutes;
        let prerenderFallbackMode;
        let appConfig = {};
        let isClientComponent = false;
        const pathIsEdgeRuntime = (0, _isedgeruntime.isEdgeRuntime)(pageRuntime);
        if (pathIsEdgeRuntime) {
            const runtime = await (0, _sandbox.getRuntimeContext)({
                paths: edgeInfo.files.map((file)=>_path.default.join(distDir, file)),
                edgeFunctionEntry: {
                    ...edgeInfo,
                    wasm: (edgeInfo.wasm ?? []).map((binding)=>({
                            ...binding,
                            filePath: _path.default.join(distDir, binding.filePath)
                        }))
                },
                name: edgeInfo.name,
                useCache: true,
                distDir
            });
            const mod = (await runtime.context._ENTRIES[`middleware_${edgeInfo.name}`]).ComponentMod;
            // This is not needed during require.
            const buildManifest = {};
            isClientComponent = (0, _clientreference.isClientReference)(mod);
            componentsResult = {
                Component: mod.default,
                Document: mod.Document,
                App: mod.App,
                routeModule: mod.routeModule,
                page,
                ComponentMod: mod,
                pageConfig: mod.config || {},
                buildManifest,
                reactLoadableManifest: {},
                getServerSideProps: mod.getServerSideProps,
                getStaticPaths: mod.getStaticPaths,
                getStaticProps: mod.getStaticProps
            };
        } else {
            componentsResult = await (0, _loadcomponents.loadComponents)({
                distDir,
                page: originalAppPath || page,
                isAppPath: pageType === 'app'
            });
        }
        const Comp = componentsResult.Component;
        let staticPathsResult;
        const routeModule = componentsResult.routeModule;
        let isRoutePPREnabled = false;
        if (pageType === 'app') {
            const ComponentMod = componentsResult.ComponentMod;
            isClientComponent = (0, _clientreference.isClientReference)(componentsResult.ComponentMod);
            let segments;
            try {
                segments = await (0, _appsegments.collectSegments)(componentsResult);
            } catch (err) {
                throw new Error(`Failed to collect configuration for ${page}`, {
                    cause: err
                });
            }
            appConfig = reduceAppConfig(await (0, _appsegments.collectSegments)(componentsResult));
            if (appConfig.dynamic === 'force-static' && pathIsEdgeRuntime) {
                _log.warn(`Page "${page}" is using runtime = 'edge' which is currently incompatible with dynamic = 'force-static'. Please remove either "runtime" or "force-static" for correct behavior`);
            }
            // A page supports partial prerendering if it is an app page and either
            // the whole app has PPR enabled or this page has PPR enabled when we're
            // in incremental mode.
            isRoutePPREnabled = routeModule.definition.kind === _routekind.RouteKind.APP_PAGE && !(0, _interceptionroutes.isInterceptionRouteAppPath)(page) && (0, _ppr.checkIsRoutePPREnabled)(pprConfig, appConfig);
            // If force dynamic was set and we don't have PPR enabled, then set the
            // revalidate to 0.
            // TODO: (PPR) remove this once PPR is enabled by default
            if (appConfig.dynamic === 'force-dynamic' && !isRoutePPREnabled) {
                appConfig.revalidate = 0;
            }
            if ((0, _isdynamic.isDynamicRoute)(page)) {
                ;
                ({ fallbackMode: prerenderFallbackMode, prerenderedRoutes } = await buildAppStaticPaths({
                    dir,
                    page,
                    dynamicIO,
                    authInterrupts,
                    configFileName,
                    segments,
                    distDir,
                    requestHeaders: {},
                    isrFlushToDisk,
                    maxMemoryCacheSize,
                    cacheHandler,
                    cacheLifeProfiles,
                    ComponentMod,
                    nextConfigOutput,
                    isRoutePPREnabled,
                    buildId
                }));
            }
        } else {
            if (!Comp || !(0, _reactis.isValidElementType)(Comp) || typeof Comp === 'string') {
                throw new Error('INVALID_DEFAULT_EXPORT');
            }
        }
        const hasGetInitialProps = !!(Comp == null ? void 0 : Comp.getInitialProps);
        const hasStaticProps = !!componentsResult.getStaticProps;
        const hasStaticPaths = !!componentsResult.getStaticPaths;
        const hasServerProps = !!componentsResult.getServerSideProps;
        // A page cannot be prerendered _and_ define a data requirement. That's
        // contradictory!
        if (hasGetInitialProps && hasStaticProps) {
            throw new Error(_constants.SSG_GET_INITIAL_PROPS_CONFLICT);
        }
        if (hasGetInitialProps && hasServerProps) {
            throw new Error(_constants.SERVER_PROPS_GET_INIT_PROPS_CONFLICT);
        }
        if (hasStaticProps && hasServerProps) {
            throw new Error(_constants.SERVER_PROPS_SSG_CONFLICT);
        }
        const pageIsDynamic = (0, _isdynamic.isDynamicRoute)(page);
        // A page cannot have static parameters if it is not a dynamic page.
        if (hasStaticProps && hasStaticPaths && !pageIsDynamic) {
            throw new Error(`getStaticPaths can only be used with dynamic pages, not '${page}'.` + `\nLearn more: https://nextjs.org/docs/routing/dynamic-routes`);
        }
        if (hasStaticProps && pageIsDynamic && !hasStaticPaths) {
            throw new Error(`getStaticPaths is required for dynamic SSG pages and is missing for '${page}'.` + `\nRead more: https://nextjs.org/docs/messages/invalid-getstaticpaths-value`);
        }
        if (hasStaticProps && hasStaticPaths || staticPathsResult) {
            ;
            ({ fallbackMode: prerenderFallbackMode, prerenderedRoutes } = await buildStaticPaths({
                page,
                locales,
                defaultLocale,
                configFileName,
                staticPathsResult,
                getStaticPaths: componentsResult.getStaticPaths
            }));
        }
        const isNextImageImported = globalThis.__NEXT_IMAGE_IMPORTED;
        const config = isClientComponent ? {} : componentsResult.pageConfig;
        let isStatic = false;
        if (!hasStaticProps && !hasGetInitialProps && !hasServerProps) {
            isStatic = true;
        }
        // When PPR is enabled, any route may be completely static, so
        // mark this route as static.
        if (isRoutePPREnabled) {
            isStatic = true;
        }
        return {
            isStatic,
            isRoutePPREnabled,
            isHybridAmp: config.amp === 'hybrid',
            isAmpOnly: config.amp === true,
            prerenderFallbackMode,
            prerenderedRoutes,
            hasStaticProps,
            hasServerProps,
            isNextImageImported,
            appConfig
        };
    }).catch((err)=>{
        if (err.message === 'INVALID_DEFAULT_EXPORT') {
            throw err;
        }
        console.error(err);
        throw new Error(`Failed to collect page data for ${page}`);
    });
}
function reduceAppConfig(segments) {
    const config = {};
    for (const segment of segments){
        const { dynamic, fetchCache, preferredRegion, revalidate, experimental_ppr, runtime, maxDuration } = segment.config || {};
        // TODO: should conflicting configs here throw an error
        // e.g. if layout defines one region but page defines another
        if (typeof preferredRegion !== 'undefined') {
            config.preferredRegion = preferredRegion;
        }
        if (typeof dynamic !== 'undefined') {
            config.dynamic = dynamic;
        }
        if (typeof fetchCache !== 'undefined') {
            config.fetchCache = fetchCache;
        }
        if (typeof revalidate !== 'undefined') {
            config.revalidate = revalidate;
        }
        // Any revalidate number overrides false, and shorter revalidate overrides
        // longer (initially).
        if (typeof revalidate === 'number' && (typeof config.revalidate !== 'number' || revalidate < config.revalidate)) {
            config.revalidate = revalidate;
        }
        // If partial prerendering has been set, only override it if the current
        // value is provided as it's resolved from root layout to leaf page.
        if (typeof experimental_ppr !== 'undefined') {
            config.experimental_ppr = experimental_ppr;
        }
        if (typeof runtime !== 'undefined') {
            config.runtime = runtime;
        }
        if (typeof maxDuration !== 'undefined') {
            config.maxDuration = maxDuration;
        }
    }
    return config;
}
async function hasCustomGetInitialProps({ page, distDir, runtimeEnvConfig, checkingApp }) {
    require('../shared/lib/runtime-config.external').setConfig(runtimeEnvConfig);
    const components = await (0, _loadcomponents.loadComponents)({
        distDir,
        page: page,
        isAppPath: false
    });
    let mod = components.ComponentMod;
    if (checkingApp) {
        mod = await mod._app || mod.default || mod;
    } else {
        mod = mod.default || mod;
    }
    mod = await mod;
    return mod.getInitialProps !== mod.origGetInitialProps;
}
async function getDefinedNamedExports({ page, distDir, runtimeEnvConfig }) {
    require('../shared/lib/runtime-config.external').setConfig(runtimeEnvConfig);
    const components = await (0, _loadcomponents.loadComponents)({
        distDir,
        page: page,
        isAppPath: false
    });
    return Object.keys(components.ComponentMod).filter((key)=>{
        return typeof components.ComponentMod[key] !== 'undefined';
    });
}
function detectConflictingPaths(combinedPages, ssgPages, additionalGeneratedSSGPaths) {
    const conflictingPaths = new Map();
    const dynamicSsgPages = [
        ...ssgPages
    ].filter((page)=>(0, _isdynamic.isDynamicRoute)(page));
    const additionalSsgPathsByPath = {};
    additionalGeneratedSSGPaths.forEach((paths, pathsPage)=>{
        additionalSsgPathsByPath[pathsPage] ||= {};
        paths.forEach((curPath)=>{
            const currentPath = curPath.toLowerCase();
            additionalSsgPathsByPath[pathsPage][currentPath] = curPath;
        });
    });
    additionalGeneratedSSGPaths.forEach((paths, pathsPage)=>{
        paths.forEach((curPath)=>{
            const lowerPath = curPath.toLowerCase();
            let conflictingPage = combinedPages.find((page)=>page.toLowerCase() === lowerPath);
            if (conflictingPage) {
                conflictingPaths.set(lowerPath, [
                    {
                        path: curPath,
                        page: pathsPage
                    },
                    {
                        path: conflictingPage,
                        page: conflictingPage
                    }
                ]);
            } else {
                let conflictingPath;
                conflictingPage = dynamicSsgPages.find((page)=>{
                    if (page === pathsPage) return false;
                    conflictingPath = additionalGeneratedSSGPaths.get(page) == null ? undefined : additionalSsgPathsByPath[page][lowerPath];
                    return conflictingPath;
                });
                if (conflictingPage && conflictingPath) {
                    conflictingPaths.set(lowerPath, [
                        {
                            path: curPath,
                            page: pathsPage
                        },
                        {
                            path: conflictingPath,
                            page: conflictingPage
                        }
                    ]);
                }
            }
        });
    });
    if (conflictingPaths.size > 0) {
        let conflictingPathsOutput = '';
        conflictingPaths.forEach((pathItems)=>{
            pathItems.forEach((pathItem, idx)=>{
                const isDynamic = pathItem.page !== pathItem.path;
                if (idx > 0) {
                    conflictingPathsOutput += 'conflicts with ';
                }
                conflictingPathsOutput += `path: "${pathItem.path}"${isDynamic ? ` from page: "${pathItem.page}" ` : ' '}`;
            });
            conflictingPathsOutput += '\n';
        });
        _log.error('Conflicting paths returned from getStaticPaths, paths must be unique per page.\n' + 'See more info here: https://nextjs.org/docs/messages/conflicting-ssg-paths\n\n' + conflictingPathsOutput);
        process.exit(1);
    }
}
async function copyTracedFiles(dir, distDir, pageKeys, appPageKeys, tracingRoot, serverConfig, middlewareManifest, hasInstrumentationHook, staticPages) {
    const outputPath = _path.default.join(distDir, 'standalone');
    let moduleType = false;
    const nextConfig = {
        ...serverConfig,
        distDir: `./${_path.default.relative(dir, distDir)}`
    };
    try {
        const packageJsonPath = _path.default.join(distDir, '../package.json');
        const packageJson = JSON.parse(await _fs.promises.readFile(packageJsonPath, 'utf8'));
        moduleType = packageJson.type === 'module';
    } catch  {}
    const copiedFiles = new Set();
    await _fs.promises.rm(outputPath, {
        recursive: true,
        force: true
    });
    async function handleTraceFiles(traceFilePath) {
        const traceData = JSON.parse(await _fs.promises.readFile(traceFilePath, 'utf8'));
        const copySema = new _asyncsema.Sema(10, {
            capacity: traceData.files.length
        });
        const traceFileDir = _path.default.dirname(traceFilePath);
        await Promise.all(traceData.files.map(async (relativeFile)=>{
            await copySema.acquire();
            const tracedFilePath = _path.default.join(traceFileDir, relativeFile);
            const fileOutputPath = _path.default.join(outputPath, _path.default.relative(tracingRoot, tracedFilePath));
            if (!copiedFiles.has(fileOutputPath)) {
                copiedFiles.add(fileOutputPath);
                await _fs.promises.mkdir(_path.default.dirname(fileOutputPath), {
                    recursive: true
                });
                const symlink = await _fs.promises.readlink(tracedFilePath).catch(()=>null);
                if (symlink) {
                    try {
                        await _fs.promises.symlink(symlink, fileOutputPath);
                    } catch (e) {
                        if (e.code !== 'EEXIST') {
                            throw e;
                        }
                    }
                } else {
                    await _fs.promises.copyFile(tracedFilePath, fileOutputPath);
                }
            }
            await copySema.release();
        }));
    }
    async function handleEdgeFunction(page) {
        var _page_wasm, _page_assets;
        async function handleFile(file) {
            const originalPath = _path.default.join(distDir, file);
            const fileOutputPath = _path.default.join(outputPath, _path.default.relative(tracingRoot, distDir), file);
            await _fs.promises.mkdir(_path.default.dirname(fileOutputPath), {
                recursive: true
            });
            await _fs.promises.copyFile(originalPath, fileOutputPath);
        }
        await Promise.all([
            page.files.map(handleFile),
            (_page_wasm = page.wasm) == null ? void 0 : _page_wasm.map((file)=>handleFile(file.filePath)),
            (_page_assets = page.assets) == null ? void 0 : _page_assets.map((file)=>handleFile(file.filePath))
        ]);
    }
    const edgeFunctionHandlers = [];
    for (const middleware of Object.values(middlewareManifest.middleware)){
        if (isMiddlewareFilename(middleware.name)) {
            edgeFunctionHandlers.push(handleEdgeFunction(middleware));
        }
    }
    for (const page of Object.values(middlewareManifest.functions)){
        edgeFunctionHandlers.push(handleEdgeFunction(page));
    }
    await Promise.all(edgeFunctionHandlers);
    for (const page of pageKeys){
        if (middlewareManifest.functions.hasOwnProperty(page)) {
            continue;
        }
        const route = (0, _normalizepagepath.normalizePagePath)(page);
        if (staticPages.has(route)) {
            continue;
        }
        const pageFile = _path.default.join(distDir, 'server', 'pages', `${(0, _normalizepagepath.normalizePagePath)(page)}.js`);
        const pageTraceFile = `${pageFile}.nft.json`;
        await handleTraceFiles(pageTraceFile).catch((err)=>{
            if (err.code !== 'ENOENT' || page !== '/404' && page !== '/500') {
                _log.warn(`Failed to copy traced files for ${pageFile}`, err);
            }
        });
    }
    if (appPageKeys) {
        for (const page of appPageKeys){
            if (middlewareManifest.functions.hasOwnProperty(page)) {
                continue;
            }
            const pageFile = _path.default.join(distDir, 'server', 'app', `${page}.js`);
            const pageTraceFile = `${pageFile}.nft.json`;
            await handleTraceFiles(pageTraceFile).catch((err)=>{
                _log.warn(`Failed to copy traced files for ${pageFile}`, err);
            });
        }
    }
    if (hasInstrumentationHook) {
        await handleTraceFiles(_path.default.join(distDir, 'server', 'instrumentation.js.nft.json'));
    }
    await handleTraceFiles(_path.default.join(distDir, 'next-server.js.nft.json'));
    const serverOutputPath = _path.default.join(outputPath, _path.default.relative(tracingRoot, dir), 'server.js');
    await _fs.promises.mkdir(_path.default.dirname(serverOutputPath), {
        recursive: true
    });
    await _fs.promises.writeFile(serverOutputPath, `${moduleType ? `performance.mark('next-start');
import path from 'path'
import { fileURLToPath } from 'url'
import module from 'module'
const require = module.createRequire(import.meta.url)
const __dirname = fileURLToPath(new URL('.', import.meta.url))
` : `const path = require('path')`}

const dir = path.join(__dirname)

process.env.NODE_ENV = 'production'
process.chdir(__dirname)

const currentPort = parseInt(process.env.PORT, 10) || 3000
const hostname = process.env.HOSTNAME || '0.0.0.0'

let keepAliveTimeout = parseInt(process.env.KEEP_ALIVE_TIMEOUT, 10)
const nextConfig = ${JSON.stringify(nextConfig)}

process.env.__NEXT_PRIVATE_STANDALONE_CONFIG = JSON.stringify(nextConfig)

require('next')
const { startServer } = require('next/dist/server/lib/start-server')

if (
  Number.isNaN(keepAliveTimeout) ||
  !Number.isFinite(keepAliveTimeout) ||
  keepAliveTimeout < 0
) {
  keepAliveTimeout = undefined
}

startServer({
  dir,
  isDev: false,
  config: nextConfig,
  hostname,
  port: currentPort,
  allowRetry: false,
  keepAliveTimeout,
}).catch((err) => {
  console.error(err);
  process.exit(1);
});`);
}
function isReservedPage(page) {
    return RESERVED_PAGE.test(page);
}
function isAppBuiltinNotFoundPage(page) {
    return /next[\\/]dist[\\/]client[\\/]components[\\/]not-found-error/.test(page);
}
function isCustomErrorPage(page) {
    return page === '/404' || page === '/500';
}
function isMiddlewareFile(file) {
    return file === `/${_constants.MIDDLEWARE_FILENAME}` || file === `/src/${_constants.MIDDLEWARE_FILENAME}`;
}
function isInstrumentationHookFile(file) {
    return file === `/${_constants.INSTRUMENTATION_HOOK_FILENAME}` || file === `/src/${_constants.INSTRUMENTATION_HOOK_FILENAME}`;
}
function getPossibleInstrumentationHookFilenames(folder, extensions) {
    const files = [];
    for (const extension of extensions){
        files.push(_path.default.join(folder, `${_constants.INSTRUMENTATION_HOOK_FILENAME}.${extension}`), _path.default.join(folder, `src`, `${_constants.INSTRUMENTATION_HOOK_FILENAME}.${extension}`));
    }
    return files;
}
function getPossibleMiddlewareFilenames(folder, extensions) {
    return extensions.map((extension)=>_path.default.join(folder, `${_constants.MIDDLEWARE_FILENAME}.${extension}`));
}
class NestedMiddlewareError extends Error {
    constructor(nestedFileNames, mainDir, pagesOrAppDir){
        super(`Nested Middleware is not allowed, found:\n` + `${nestedFileNames.map((file)=>`pages${file}`).join('\n')}\n` + `Please move your code to a single file at ${_path.default.join(_path.default.posix.sep, _path.default.relative(mainDir, _path.default.resolve(pagesOrAppDir, '..')), 'middleware')} instead.\n` + `Read More - https://nextjs.org/docs/messages/nested-middleware`);
    }
}
function getSupportedBrowsers(dir, isDevelopment) {
    let browsers;
    try {
        const browsersListConfig = _browserslist.default.loadConfig({
            path: dir,
            env: isDevelopment ? 'development' : 'production'
        });
        // Running `browserslist` resolves `extends` and other config features into a list of browsers
        if (browsersListConfig && browsersListConfig.length > 0) {
            browsers = (0, _browserslist.default)(browsersListConfig);
        }
    } catch  {}
    // When user has browserslist use that target
    if (browsers && browsers.length > 0) {
        return browsers;
    }
    // Uses modern browsers as the default.
    return _constants1.MODERN_BROWSERSLIST_TARGET;
}
function isWebpackServerOnlyLayer(layer) {
    return Boolean(layer && _constants.WEBPACK_LAYERS.GROUP.serverOnly.includes(layer));
}
function isWebpackClientOnlyLayer(layer) {
    return Boolean(layer && _constants.WEBPACK_LAYERS.GROUP.clientOnly.includes(layer));
}
function isWebpackDefaultLayer(layer) {
    return layer === null || layer === undefined;
}
function isWebpackBundledLayer(layer) {
    return Boolean(layer && _constants.WEBPACK_LAYERS.GROUP.bundled.includes(layer));
}
function isWebpackAppPagesLayer(layer) {
    return Boolean(layer && _constants.WEBPACK_LAYERS.GROUP.appPages.includes(layer));
}
function collectMeta({ status, headers }) {
    const meta = {};
    if (status !== 200) {
        meta.status = status;
    }
    if (headers && Object.keys(headers).length) {
        meta.headers = {};
        // normalize header values as initialHeaders
        // must be Record<string, string>
        for(const key in headers){
            // set-cookie is already handled - the middleware cookie setting case
            // isn't needed for the prerender manifest since it can't read cookies
            if (key === 'x-middleware-set-cookie') continue;
            let value = headers[key];
            if (Array.isArray(value)) {
                if (key === 'set-cookie') {
                    value = value.join(',');
                } else {
                    value = value[value.length - 1];
                }
            }
            if (typeof value === 'string') {
                meta.headers[key] = value;
            }
        }
    }
    return meta;
}

//# sourceMappingURL=utils.js.map