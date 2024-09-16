import "../server/require-hook";
import "../server/node-polyfill-crypto";
import "../server/node-environment";
import { green, yellow, red, cyan, white, bold, underline } from "../lib/picocolors";
import getGzipSize from "next/dist/compiled/gzip-size";
import textTable from "next/dist/compiled/text-table";
import path from "path";
import { promises as fs } from "fs";
import { isValidElementType } from "next/dist/compiled/react-is";
import stripAnsi from "next/dist/compiled/strip-ansi";
import browserslist from "next/dist/compiled/browserslist";
import { SSG_GET_INITIAL_PROPS_CONFLICT, SERVER_PROPS_GET_INIT_PROPS_CONFLICT, SERVER_PROPS_SSG_CONFLICT, MIDDLEWARE_FILENAME, INSTRUMENTATION_HOOK_FILENAME, WEBPACK_LAYERS } from "../lib/constants";
import { MODERN_BROWSERSLIST_TARGET, UNDERSCORE_NOT_FOUND_ROUTE } from "../shared/lib/constants";
import prettyBytes from "../lib/pretty-bytes";
import { getRouteRegex } from "../shared/lib/router/utils/route-regex";
import { getRouteMatcher } from "../shared/lib/router/utils/route-matcher";
import { isDynamicRoute } from "../shared/lib/router/utils/is-dynamic";
import escapePathDelimiters from "../shared/lib/router/utils/escape-path-delimiters";
import { findPageFile } from "../server/lib/find-page-file";
import { removeTrailingSlash } from "../shared/lib/router/utils/remove-trailing-slash";
import { isEdgeRuntime } from "../lib/is-edge-runtime";
import { normalizeLocalePath } from "../shared/lib/i18n/normalize-locale-path";
import * as Log from "./output/log";
import { loadComponents } from "../server/load-components";
import { trace } from "../trace";
import { setHttpClientAndAgentOptions } from "../server/setup-http-agent-env";
import { Sema } from "next/dist/compiled/async-sema";
import { denormalizePagePath } from "../shared/lib/page-path/denormalize-page-path";
import { normalizePagePath } from "../shared/lib/page-path/normalize-page-path";
import { getRuntimeContext } from "../server/web/sandbox";
import { isClientReference } from "../lib/client-reference";
import { StaticGenerationAsyncStorageWrapper } from "../server/async-storage/static-generation-async-storage-wrapper";
import { IncrementalCache } from "../server/lib/incremental-cache";
import { nodeFs } from "../server/lib/node-fs-methods";
import * as ciEnvironment from "../telemetry/ci-info";
import { normalizeAppPath } from "../shared/lib/router/utils/app-paths";
import { denormalizeAppPagePath } from "../shared/lib/page-path/denormalize-app-path";
import { RouteKind } from "../server/future/route-kind";
import { isAppRouteRouteModule } from "../server/future/route-modules/checks";
import { interopDefault } from "../lib/interop-default";
import { formatDynamicImportPath } from "../lib/format-dynamic-import-path";
import { isInterceptionRouteAppPath } from "../server/future/helpers/interception-routes";
// Use `print()` for expected console output
const print = console.log;
const RESERVED_PAGE = /^\/(_app|_error|_document|api(\/|$))/;
const fileGzipStats = {};
const fsStatGzip = (file)=>{
    const cached = fileGzipStats[file];
    if (cached) return cached;
    return fileGzipStats[file] = getGzipSize.file(file);
};
const fileSize = async (file)=>(await fs.stat(file)).size;
const fileStats = {};
const fsStat = (file)=>{
    const cached = fileStats[file];
    if (cached) return cached;
    return fileStats[file] = fileSize(file);
};
export function unique(main, sub) {
    return [
        ...new Set([
            ...main,
            ...sub
        ])
    ];
}
export function difference(main, sub) {
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
export async function computeFromManifest(manifests, distPath, gzipSize = true, pageInfos) {
    var _manifests_app, _files_app;
    if (Object.is(cachedBuildManifest, manifests.build) && lastComputePageInfo === !!pageInfos && Object.is(cachedAppBuildManifest, manifests.app)) {
        return lastCompute;
    }
    // Determine the files that are in pages and app and count them, this will
    // tell us if they are unique or common.
    const countBuildFiles = (map, key, manifest)=>{
        for (const file of manifest[key]){
            if (key === "/_app") {
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
            stats.set(f, await getSize(path.join(distPath, f)));
        } catch  {}
    }));
    const groupFiles = async (listing)=>{
        const entries = [
            ...listing.each.entries()
        ];
        const shapeGroup = (group)=>group.reduce((acc, [f])=>{
                acc.files.push(f);
                const size = stats.get(f);
                if (typeof size === "number") {
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
export function isMiddlewareFilename(file) {
    return file === MIDDLEWARE_FILENAME || file === `src/${MIDDLEWARE_FILENAME}`;
}
export function isInstrumentationHookFilename(file) {
    return file === INSTRUMENTATION_HOOK_FILENAME || file === `src/${INSTRUMENTATION_HOOK_FILENAME}`;
}
const filterAndSortList = (list, routeType, hasCustomApp)=>{
    let pages;
    if (routeType === "app") {
        // filter out static app route of /favicon.ico
        pages = list.filter((e)=>e !== "/favicon.ico");
    } else {
        // filter built-in pages
        pages = list.slice().filter((e)=>!(e === "/_document" || e === "/_error" || !hasCustomApp && e === "/_app"));
    }
    return pages.sort((a, b)=>a.localeCompare(b));
};
export function serializePageInfos(input) {
    return Array.from(input.entries());
}
export function deserializePageInfos(input) {
    return new Map(input);
}
export async function printTreeView(lists, pageInfos, { distPath, buildId, pagesDir, pageExtensions, buildManifest, appBuildManifest, middlewareManifest, useStaticPages404, gzipSize = true }) {
    var _lists_app, _middlewareManifest_middleware;
    const getPrettySize = (_size)=>{
        const size = prettyBytes(_size);
        return white(bold(size));
    };
    const MIN_DURATION = 300;
    const getPrettyDuration = (_duration)=>{
        const duration = `${_duration} ms`;
        // green for 300-1000ms
        if (_duration < 1000) return green(duration);
        // yellow for 1000-2000ms
        if (_duration < 2000) return yellow(duration);
        // red for >= 2000ms
        return red(bold(duration));
    };
    const getCleanName = (fileName)=>fileName// Trim off `static/`
        .replace(/^static\//, "")// Re-add `static/` for root files
        .replace(/^<buildId>/, "static")// Remove file hash
        .replace(/(?:^|[.-])([0-9a-z]{6})[0-9a-z]{14}(?=\.)/, ".$1");
    // Check if we have a custom app.
    const hasCustomApp = !!(pagesDir && await findPageFile(pagesDir, "/_app", pageExtensions, false));
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
            routerType === "app" ? "Route (app)" : "Route (pages)",
            "Size",
            "First Load JS"
        ].map((entry)=>underline(entry)));
        filteredPages.forEach((item, i, arr)=>{
            var _pageInfo_ssgPageDurations, _buildManifest_pages_item, _pageInfo_ssgPageRoutes;
            const border = i === 0 ? arr.length === 1 ? "─" : "┌" : i === arr.length - 1 ? "└" : "├";
            const pageInfo = pageInfos.get(item);
            const ampFirst = buildManifest.ampFirstPages.includes(item);
            const totalDuration = ((pageInfo == null ? void 0 : pageInfo.pageDuration) || 0) + ((pageInfo == null ? void 0 : (_pageInfo_ssgPageDurations = pageInfo.ssgPageDurations) == null ? void 0 : _pageInfo_ssgPageDurations.reduce((a, b)=>a + (b || 0), 0)) || 0);
            let symbol;
            if (item === "/_app" || item === "/_app.server") {
                symbol = " ";
            } else if (isEdgeRuntime(pageInfo == null ? void 0 : pageInfo.runtime)) {
                symbol = "ƒ";
            } else if (pageInfo == null ? void 0 : pageInfo.isPPR) {
                if (// If the page has an empty prelude, then it's equivalent to a dynamic page
                (pageInfo == null ? void 0 : pageInfo.hasEmptyPrelude) || // ensure we don't mark dynamic paths that postponed as being dynamic
                // since in this case we're able to partially prerender it
                pageInfo.isDynamicAppRoute && !pageInfo.hasPostponed) {
                    symbol = "ƒ";
                } else if (!(pageInfo == null ? void 0 : pageInfo.hasPostponed)) {
                    symbol = "○";
                } else {
                    symbol = "◐";
                }
            } else if (pageInfo == null ? void 0 : pageInfo.isStatic) {
                symbol = "○";
            } else if (pageInfo == null ? void 0 : pageInfo.isSSG) {
                symbol = "●";
            } else {
                symbol = "ƒ";
            }
            usedSymbols.add(symbol);
            if (pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds) usedSymbols.add("ISR");
            messages.push([
                `${border} ${symbol} ${(pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds) ? `${item} (ISR: ${pageInfo == null ? void 0 : pageInfo.initialRevalidateSeconds} Seconds)` : item}${totalDuration > MIN_DURATION ? ` (${getPrettyDuration(totalDuration)})` : ""}`,
                pageInfo ? ampFirst ? cyan("AMP") : pageInfo.size >= 0 ? prettyBytes(pageInfo.size) : "" : "",
                pageInfo ? ampFirst ? cyan("AMP") : pageInfo.size >= 0 ? getPrettySize(pageInfo.totalSize) : "" : ""
            ]);
            const uniqueCssFiles = ((_buildManifest_pages_item = buildManifest.pages[item]) == null ? void 0 : _buildManifest_pages_item.filter((file)=>{
                var _stats_router_routerType;
                return file.endsWith(".css") && ((_stats_router_routerType = stats.router[routerType]) == null ? void 0 : _stats_router_routerType.unique.files.includes(file));
            })) || [];
            if (uniqueCssFiles.length > 0) {
                const contSymbol = i === arr.length - 1 ? " " : "├";
                uniqueCssFiles.forEach((file, index, { length })=>{
                    const innerSymbol = index === length - 1 ? "└" : "├";
                    const size = stats.sizes.get(file);
                    messages.push([
                        `${contSymbol}   ${innerSymbol} ${getCleanName(file)}`,
                        typeof size === "number" ? prettyBytes(size) : "",
                        ""
                    ]);
                });
            }
            if (pageInfo == null ? void 0 : (_pageInfo_ssgPageRoutes = pageInfo.ssgPageRoutes) == null ? void 0 : _pageInfo_ssgPageRoutes.length) {
                const totalRoutes = pageInfo.ssgPageRoutes.length;
                const contSymbol = i === arr.length - 1 ? " " : "├";
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
                    const innerSymbol = index === length - 1 ? "└" : "├";
                    messages.push([
                        `${contSymbol}   ${innerSymbol} ${route}${duration > MIN_DURATION ? ` (${getPrettyDuration(duration)})` : ""}${avgDuration && avgDuration > MIN_DURATION ? ` (avg ${getPrettyDuration(avgDuration)})` : ""}`,
                        "",
                        ""
                    ]);
                });
            }
        });
        const sharedFilesSize = (_stats_router_routerType = stats.router[routerType]) == null ? void 0 : _stats_router_routerType.common.size.total;
        const sharedFiles = ((_stats_router_routerType1 = stats.router[routerType]) == null ? void 0 : _stats_router_routerType1.common.files) ?? [];
        messages.push([
            "+ First Load JS shared by all",
            typeof sharedFilesSize === "number" ? getPrettySize(sharedFilesSize) : "",
            ""
        ]);
        const sharedCssFiles = [];
        const sharedJsChunks = [
            ...sharedFiles.filter((file)=>{
                if (file.endsWith(".css")) {
                    sharedCssFiles.push(file);
                    return false;
                }
                return true;
            }).map((e)=>e.replace(buildId, "<buildId>")).sort(),
            ...sharedCssFiles.map((e)=>e.replace(buildId, "<buildId>")).sort()
        ];
        // if the some chunk are less than 10kb or we don't know the size, we only show the total size of the rest
        const tenKbLimit = 10 * 1000;
        let restChunkSize = 0;
        let restChunkCount = 0;
        sharedJsChunks.forEach((fileName, index, { length })=>{
            const innerSymbol = index + restChunkCount === length - 1 ? "└" : "├";
            const originalName = fileName.replace("<buildId>", buildId);
            const cleanName = getCleanName(fileName);
            const size = stats.sizes.get(originalName);
            if (!size || size < tenKbLimit) {
                restChunkCount++;
                restChunkSize += size || 0;
                return;
            }
            messages.push([
                `  ${innerSymbol} ${cleanName}`,
                prettyBytes(size),
                ""
            ]);
        });
        if (restChunkCount > 0) {
            messages.push([
                `  └ other shared chunks (total)`,
                prettyBytes(restChunkSize),
                ""
            ]);
        }
    };
    // If enabled, then print the tree for the app directory.
    if (lists.app && stats.router.app) {
        await printFileTree({
            routerType: "app",
            list: lists.app
        });
        messages.push([
            "",
            "",
            ""
        ]);
    }
    pageInfos.set("/404", {
        ...pageInfos.get("/404") || pageInfos.get("/_error"),
        isStatic: useStaticPages404
    });
    // If there's no app /_notFound page present, then the 404 is still using the pages/404
    if (!lists.pages.includes("/404") && !((_lists_app = lists.app) == null ? void 0 : _lists_app.includes(UNDERSCORE_NOT_FOUND_ROUTE))) {
        lists.pages = [
            ...lists.pages,
            "/404"
        ];
    }
    // Print the tree view for the pages directory.
    await printFileTree({
        routerType: "pages",
        list: lists.pages
    });
    const middlewareInfo = (_middlewareManifest_middleware = middlewareManifest.middleware) == null ? void 0 : _middlewareManifest_middleware["/"];
    if ((middlewareInfo == null ? void 0 : middlewareInfo.files.length) > 0) {
        const middlewareSizes = await Promise.all(middlewareInfo.files.map((dep)=>`${distPath}/${dep}`).map(gzipSize ? fsStatGzip : fsStat));
        messages.push([
            "",
            "",
            ""
        ]);
        messages.push([
            "ƒ Middleware",
            getPrettySize(sum(middlewareSizes)),
            ""
        ]);
    }
    print(textTable(messages, {
        align: [
            "l",
            "l",
            "r"
        ],
        stringLength: (str)=>stripAnsi(str).length
    }));
    print();
    print(textTable([
        usedSymbols.has("○") && [
            "○",
            "(Static)",
            "prerendered as static content"
        ],
        usedSymbols.has("●") && [
            "●",
            "(SSG)",
            `prerendered as static HTML (uses ${cyan("getStaticProps")})`
        ],
        usedSymbols.has("ISR") && [
            "",
            "(ISR)",
            `incremental static regeneration (uses revalidate in ${cyan("getStaticProps")})`
        ],
        usedSymbols.has("◐") && [
            "◐",
            "(Partial Prerender)",
            "prerendered as static HTML with dynamic server-streamed content"
        ],
        usedSymbols.has("ƒ") && [
            "ƒ",
            "(Dynamic)",
            `server-rendered on demand`
        ]
    ].filter((x)=>x), {
        align: [
            "l",
            "l",
            "l"
        ],
        stringLength: (str)=>stripAnsi(str).length
    }));
    print();
}
export function printCustomRoutes({ redirects, rewrites, headers }) {
    const printRoutes = (routes, type)=>{
        const isRedirects = type === "Redirects";
        const isHeaders = type === "Headers";
        print(underline(type));
        /*
        ┌ source
        ├ permanent/statusCode
        └ destination
     */ const routesStr = routes.map((route)=>{
            let routeStr = `┌ source: ${route.source}\n`;
            if (!isHeaders) {
                const r = route;
                routeStr += `${isRedirects ? "├" : "└"} destination: ${r.destination}\n`;
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
                    routeStr += `  ${last ? "└" : "├"} ${header.key}: ${header.value}\n`;
                }
            }
            return routeStr;
        }).join("\n");
        print(`${routesStr}\n`);
    };
    print();
    if (redirects.length) {
        printRoutes(redirects, "Redirects");
    }
    if (headers.length) {
        printRoutes(headers, "Headers");
    }
    const combinedRewrites = [
        ...rewrites.beforeFiles,
        ...rewrites.afterFiles,
        ...rewrites.fallback
    ];
    if (combinedRewrites.length) {
        printRoutes(combinedRewrites, "Rewrites");
    }
}
export async function getJsPageSizeInKb(routerType, page, distPath, buildManifest, appBuildManifest, gzipSize = true, cachedStats) {
    const pageManifest = routerType === "pages" ? buildManifest : appBuildManifest;
    if (!pageManifest) {
        throw new Error('expected appBuildManifest with an "app" pageType');
    }
    // Normalize appBuildManifest keys
    if (routerType === "app") {
        pageManifest.pages = Object.entries(pageManifest.pages).reduce((acc, [key, value])=>{
            const newKey = normalizeAppPath(key);
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
    const pagePath = routerType === "pages" ? denormalizePagePath(page) : denormalizeAppPagePath(page);
    const fnFilterJs = (entry)=>entry.endsWith(".js");
    const pageFiles = (pageManifest.pages[pagePath] ?? []).filter(fnFilterJs);
    const appFiles = (pageManifest.pages["/_app"] ?? []).filter(fnFilterJs);
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
        if (typeof size !== "number") {
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
export async function buildStaticPaths({ page, getStaticPaths, staticPathsResult, configFileName, locales, defaultLocale, appDir }) {
    const prerenderPaths = new Set();
    const encodedPrerenderPaths = new Set();
    const _routeRegex = getRouteRegex(page);
    const _routeMatcher = getRouteMatcher(_routeRegex);
    // Get the default list of allowed params.
    const _validParamKeys = Object.keys(_routeMatcher(page));
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
    if (!staticPathsResult || typeof staticPathsResult !== "object" || Array.isArray(staticPathsResult)) {
        throw new Error(`Invalid value returned from getStaticPaths in ${page}. Received ${typeof staticPathsResult} ${expectedReturnVal}`);
    }
    const invalidStaticPathKeys = Object.keys(staticPathsResult).filter((key)=>!(key === "paths" || key === "fallback"));
    if (invalidStaticPathKeys.length > 0) {
        throw new Error(`Extra keys returned from getStaticPaths in ${page} (${invalidStaticPathKeys.join(", ")}) ${expectedReturnVal}`);
    }
    if (!(typeof staticPathsResult.fallback === "boolean" || staticPathsResult.fallback === "blocking")) {
        throw new Error(`The \`fallback\` key must be returned from getStaticPaths in ${page}.\n` + expectedReturnVal);
    }
    const toPrerender = staticPathsResult.paths;
    if (!Array.isArray(toPrerender)) {
        throw new Error(`Invalid \`paths\` value returned from getStaticPaths in ${page}.\n` + `\`paths\` must be an array of strings or objects of shape { params: [key: string]: string }`);
    }
    toPrerender.forEach((entry)=>{
        // For a string-provided path, we must make sure it matches the dynamic
        // route.
        if (typeof entry === "string") {
            entry = removeTrailingSlash(entry);
            const localePathResult = normalizeLocalePath(entry, locales);
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
            prerenderPaths.add(entry.split("/").map((segment)=>escapePathDelimiters(decodeURIComponent(segment), true)).join("/"));
            encodedPrerenderPaths.add(entry);
        } else {
            const invalidKeys = Object.keys(entry).filter((key)=>key !== "params" && key !== "locale");
            if (invalidKeys.length) {
                throw new Error(`Additional keys were returned from \`getStaticPaths\` in page "${page}". ` + `URL Parameters intended for this dynamic route must be nested under the \`params\` key, i.e.:` + `\n\n\treturn { params: { ${_validParamKeys.map((k)=>`${k}: ...`).join(", ")} } }` + `\n\nKeys that need to be moved: ${invalidKeys.join(", ")}.\n`);
            }
            const { params = {} } = entry;
            let builtPage = page;
            let encodedBuiltPage = page;
            _validParamKeys.forEach((validParamKey)=>{
                const { repeat, optional } = _routeRegex.groups[validParamKey];
                let paramValue = params[validParamKey];
                if (optional && params.hasOwnProperty(validParamKey) && (paramValue === null || paramValue === undefined || paramValue === false)) {
                    paramValue = [];
                }
                if (repeat && !Array.isArray(paramValue) || !repeat && typeof paramValue !== "string") {
                    // If from appDir and not all params were provided from
                    // generateStaticParams we can just filter this entry out
                    // as it's meant to be generated at runtime
                    if (appDir && typeof paramValue === "undefined") {
                        builtPage = "";
                        encodedBuiltPage = "";
                        return;
                    }
                    throw new Error(`A required parameter (${validParamKey}) was not provided as ${repeat ? "an array" : "a string"} received ${typeof paramValue} in ${appDir ? "generateStaticParams" : "getStaticPaths"} for ${page}`);
                }
                let replaced = `[${repeat ? "..." : ""}${validParamKey}]`;
                if (optional) {
                    replaced = `[${replaced}]`;
                }
                builtPage = builtPage.replace(replaced, repeat ? paramValue.map((segment)=>escapePathDelimiters(segment, true)).join("/") : escapePathDelimiters(paramValue, true)).replace(/\\/g, "/").replace(/(?!^)\/$/, "");
                encodedBuiltPage = encodedBuiltPage.replace(replaced, repeat ? paramValue.map(encodeURIComponent).join("/") : encodeURIComponent(paramValue)).replace(/\\/g, "/").replace(/(?!^)\/$/, "");
            });
            if (!builtPage && !encodedBuiltPage) {
                return;
            }
            if (entry.locale && !(locales == null ? void 0 : locales.includes(entry.locale))) {
                throw new Error(`Invalid locale returned from getStaticPaths for ${page}, the locale ${entry.locale} is not specified in ${configFileName}`);
            }
            const curLocale = entry.locale || defaultLocale || "";
            prerenderPaths.add(`${curLocale ? `/${curLocale}` : ""}${curLocale && builtPage === "/" ? "" : builtPage}`);
            encodedPrerenderPaths.add(`${curLocale ? `/${curLocale}` : ""}${curLocale && encodedBuiltPage === "/" ? "" : encodedBuiltPage}`);
        }
    });
    return {
        paths: [
            ...prerenderPaths
        ],
        fallback: staticPathsResult.fallback,
        encodedPaths: [
            ...encodedPrerenderPaths
        ]
    };
}
export const collectAppConfig = (mod)=>{
    let hasConfig = false;
    const config = {};
    if (typeof (mod == null ? void 0 : mod.revalidate) !== "undefined") {
        config.revalidate = mod.revalidate;
        hasConfig = true;
    }
    if (typeof (mod == null ? void 0 : mod.dynamicParams) !== "undefined") {
        config.dynamicParams = mod.dynamicParams;
        hasConfig = true;
    }
    if (typeof (mod == null ? void 0 : mod.dynamic) !== "undefined") {
        config.dynamic = mod.dynamic;
        hasConfig = true;
    }
    if (typeof (mod == null ? void 0 : mod.fetchCache) !== "undefined") {
        config.fetchCache = mod.fetchCache;
        hasConfig = true;
    }
    if (typeof (mod == null ? void 0 : mod.preferredRegion) !== "undefined") {
        config.preferredRegion = mod.preferredRegion;
        hasConfig = true;
    }
    return hasConfig ? config : undefined;
};
/**
 * Walks the loader tree and collects the generate parameters for each segment.
 *
 * @param tree the loader tree
 * @returns the generate parameters for each segment
 */ export async function collectGenerateParams(tree) {
    const generateParams = [];
    const parentSegments = [];
    let currentLoaderTree = tree;
    while(currentLoaderTree){
        var _components_layout_, _components_layout, _components_page_, _components_page;
        const [// TODO: check if this is ever undefined
        page = "", parallelRoutes, components] = currentLoaderTree;
        // If the segment doesn't have any components, then skip it.
        if (!components) continue;
        const isLayout = !!components.layout;
        const mod = await (isLayout ? (_components_layout = components.layout) == null ? void 0 : (_components_layout_ = _components_layout[0]) == null ? void 0 : _components_layout_.call(_components_layout) : (_components_page = components.page) == null ? void 0 : (_components_page_ = _components_page[0]) == null ? void 0 : _components_page_.call(_components_page));
        if (page) {
            parentSegments.push(page);
        }
        const config = mod ? collectAppConfig(mod) : undefined;
        const isClientComponent = isClientReference(mod);
        const isDynamicSegment = /^\[.+\]$/.test(page);
        const { generateStaticParams, getStaticPaths } = mod || {};
        if (isDynamicSegment && isClientComponent && generateStaticParams) {
            throw new Error(`Page "${page}" cannot export "generateStaticParams()" because it is a client component`);
        }
        const segmentPath = `/${parentSegments.join("/")}${page && parentSegments.length > 0 ? "/" : ""}${page}`;
        const result = {
            isLayout,
            isDynamicSegment,
            segmentPath,
            config,
            getStaticPaths: !isClientComponent ? getStaticPaths : undefined,
            generateStaticParams: !isClientComponent ? generateStaticParams : undefined
        };
        // If the configuration contributes to the static generation, then add it
        // to the list.
        if (result.config || result.generateStaticParams || result.getStaticPaths || isDynamicSegment) {
            generateParams.push(result);
        }
        // Use this route's parallel route children as the next segment.
        currentLoaderTree = parallelRoutes.children;
    }
    return generateParams;
}
export async function buildAppStaticPaths({ dir, page, distDir, configFileName, generateParams, isrFlushToDisk, cacheHandler, requestHeaders, maxMemoryCacheSize, fetchCacheKeyPrefix, ppr, ComponentMod }) {
    ComponentMod.patchFetch();
    let CacheHandler;
    if (cacheHandler) {
        CacheHandler = interopDefault(await import(formatDynamicImportPath(dir, cacheHandler)).then((mod)=>mod.default || mod));
    }
    const incrementalCache = new IncrementalCache({
        fs: nodeFs,
        dev: true,
        // Enabled both for build as we're only writing this cache, not reading it.
        pagesDir: true,
        appDir: true,
        flushToDisk: isrFlushToDisk,
        serverDistDir: path.join(distDir, "server"),
        fetchCacheKeyPrefix,
        maxMemoryCacheSize,
        getPrerenderManifest: ()=>({
                version: -1,
                routes: {},
                dynamicRoutes: {},
                notFoundRoutes: [],
                preview: null
            }),
        CurCacheHandler: CacheHandler,
        requestHeaders,
        minimalMode: ciEnvironment.hasNextSupport,
        experimental: {
            ppr
        }
    });
    return StaticGenerationAsyncStorageWrapper.wrap(ComponentMod.staticGenerationAsyncStorage, {
        urlPathname: page,
        renderOpts: {
            originalPathname: page,
            incrementalCache,
            supportsDynamicHTML: true,
            isRevalidate: false,
            // building static paths should never postpone
            experimental: {
                ppr: false
            }
        }
    }, async ()=>{
        const pageEntry = generateParams[generateParams.length - 1];
        // if the page has legacy getStaticPaths we call it like normal
        if (typeof (pageEntry == null ? void 0 : pageEntry.getStaticPaths) === "function") {
            return buildStaticPaths({
                page,
                configFileName,
                getStaticPaths: pageEntry.getStaticPaths
            });
        } else {
            // if generateStaticParams is being used we iterate over them
            // collecting them from each level
            let hadAllParamsGenerated = false;
            const buildParams = async (paramsItems = [
                {}
            ], idx = 0)=>{
                const curGenerate = generateParams[idx];
                if (idx === generateParams.length) {
                    return paramsItems;
                }
                if (typeof curGenerate.generateStaticParams !== "function" && idx < generateParams.length) {
                    if (curGenerate.isDynamicSegment) {
                        // This dynamic level has no generateStaticParams so we change
                        // this flag to false, but it could be covered by a later
                        // generateStaticParams so it could be set back to true.
                        hadAllParamsGenerated = false;
                    }
                    return buildParams(paramsItems, idx + 1);
                }
                hadAllParamsGenerated = true;
                const newParams = [];
                if (curGenerate.generateStaticParams) {
                    for (const params of paramsItems){
                        const result = await curGenerate.generateStaticParams({
                            params
                        });
                        // TODO: validate the result is valid here or wait for buildStaticPaths to validate?
                        for (const item of result){
                            newParams.push({
                                ...params,
                                ...item
                            });
                        }
                    }
                }
                if (idx < generateParams.length) {
                    return buildParams(newParams, idx + 1);
                }
                return newParams;
            };
            const builtParams = await buildParams();
            const fallback = !generateParams.some(// TODO: dynamic params should be allowed
            // to be granular per segment but we need
            // additional information stored/leveraged in
            // the prerender-manifest to allow this behavior
            (generate)=>{
                var _generate_config;
                return ((_generate_config = generate.config) == null ? void 0 : _generate_config.dynamicParams) === false;
            });
            if (!hadAllParamsGenerated) {
                return {
                    paths: undefined,
                    fallback: process.env.NODE_ENV === "production" && isDynamicRoute(page) ? true : undefined,
                    encodedPaths: undefined
                };
            }
            return buildStaticPaths({
                staticPathsResult: {
                    fallback,
                    paths: builtParams.map((params)=>({
                            params
                        }))
                },
                page,
                configFileName,
                appDir: true
            });
        }
    });
}
export async function isPageStatic({ dir, page, distDir, configFileName, runtimeEnvConfig, httpAgentOptions, locales, defaultLocale, parentId, pageRuntime, edgeInfo, pageType, originalAppPath, isrFlushToDisk, maxMemoryCacheSize, cacheHandler, ppr }) {
    const isPageStaticSpan = trace("is-page-static-utils", parentId);
    return isPageStaticSpan.traceAsyncFn(async ()=>{
        var _componentsResult_ComponentMod;
        require("../shared/lib/runtime-config.external").setConfig(runtimeEnvConfig);
        setHttpClientAndAgentOptions({
            httpAgentOptions
        });
        let componentsResult;
        let prerenderRoutes;
        let encodedPrerenderRoutes;
        let prerenderFallback;
        let appConfig = {};
        let isClientComponent = false;
        const pathIsEdgeRuntime = isEdgeRuntime(pageRuntime);
        if (pathIsEdgeRuntime) {
            const runtime = await getRuntimeContext({
                paths: edgeInfo.files.map((file)=>path.join(distDir, file)),
                edgeFunctionEntry: {
                    ...edgeInfo,
                    wasm: (edgeInfo.wasm ?? []).map((binding)=>({
                            ...binding,
                            filePath: path.join(distDir, binding.filePath)
                        }))
                },
                name: edgeInfo.name,
                useCache: true,
                distDir
            });
            const mod = (await runtime.context._ENTRIES[`middleware_${edgeInfo.name}`]).ComponentMod;
            isClientComponent = isClientReference(mod);
            componentsResult = {
                Component: mod.default,
                ComponentMod: mod,
                pageConfig: mod.config || {},
                // @ts-expect-error this is not needed during require
                buildManifest: {},
                reactLoadableManifest: {},
                getServerSideProps: mod.getServerSideProps,
                getStaticPaths: mod.getStaticPaths,
                getStaticProps: mod.getStaticProps
            };
        } else {
            componentsResult = await loadComponents({
                distDir,
                page: originalAppPath || page,
                isAppPath: pageType === "app"
            });
        }
        const Comp = componentsResult.Component;
        let staticPathsResult;
        const routeModule = (_componentsResult_ComponentMod = componentsResult.ComponentMod) == null ? void 0 : _componentsResult_ComponentMod.routeModule;
        let supportsPPR = false;
        if (pageType === "app") {
            if (ppr && routeModule.definition.kind === RouteKind.APP_PAGE) {
                supportsPPR = true;
            }
            const ComponentMod = componentsResult.ComponentMod;
            isClientComponent = isClientReference(componentsResult.ComponentMod);
            const { tree } = ComponentMod;
            const generateParams = routeModule && isAppRouteRouteModule(routeModule) ? [
                {
                    config: {
                        revalidate: routeModule.userland.revalidate,
                        dynamic: routeModule.userland.dynamic,
                        dynamicParams: routeModule.userland.dynamicParams
                    },
                    generateStaticParams: routeModule.userland.generateStaticParams,
                    segmentPath: page
                }
            ] : await collectGenerateParams(tree);
            appConfig = generateParams.reduce((builtConfig, curGenParams)=>{
                const { dynamic, fetchCache, preferredRegion, revalidate: curRevalidate } = (curGenParams == null ? void 0 : curGenParams.config) || {};
                // TODO: should conflicting configs here throw an error
                // e.g. if layout defines one region but page defines another
                if (typeof builtConfig.preferredRegion === "undefined") {
                    builtConfig.preferredRegion = preferredRegion;
                }
                if (typeof builtConfig.dynamic === "undefined") {
                    builtConfig.dynamic = dynamic;
                }
                if (typeof builtConfig.fetchCache === "undefined") {
                    builtConfig.fetchCache = fetchCache;
                }
                // any revalidate number overrides false
                // shorter revalidate overrides longer (initially)
                if (typeof builtConfig.revalidate === "undefined") {
                    builtConfig.revalidate = curRevalidate;
                }
                if (typeof curRevalidate === "number" && (typeof builtConfig.revalidate !== "number" || curRevalidate < builtConfig.revalidate)) {
                    builtConfig.revalidate = curRevalidate;
                }
                return builtConfig;
            }, {});
            if (appConfig.dynamic === "force-static" && pathIsEdgeRuntime) {
                Log.warn(`Page "${page}" is using runtime = 'edge' which is currently incompatible with dynamic = 'force-static'. Please remove either "runtime" or "force-static" for correct behavior`);
            }
            // If force dynamic was set and we don't have PPR enabled, then set the
            // revalidate to 0.
            // TODO: (PPR) remove this once PPR is enabled by default
            if (appConfig.dynamic === "force-dynamic" && !supportsPPR) {
                appConfig.revalidate = 0;
            }
            if (isDynamicRoute(page)) {
                ({ paths: prerenderRoutes, fallback: prerenderFallback, encodedPaths: encodedPrerenderRoutes } = await buildAppStaticPaths({
                    dir,
                    page,
                    configFileName,
                    generateParams,
                    distDir,
                    requestHeaders: {},
                    isrFlushToDisk,
                    maxMemoryCacheSize,
                    cacheHandler,
                    ppr,
                    ComponentMod
                }));
            }
        } else {
            if (!Comp || !isValidElementType(Comp) || typeof Comp === "string") {
                throw new Error("INVALID_DEFAULT_EXPORT");
            }
        }
        const hasGetInitialProps = !!(Comp == null ? void 0 : Comp.getInitialProps);
        const hasStaticProps = !!componentsResult.getStaticProps;
        const hasStaticPaths = !!componentsResult.getStaticPaths;
        const hasServerProps = !!componentsResult.getServerSideProps;
        // A page cannot be prerendered _and_ define a data requirement. That's
        // contradictory!
        if (hasGetInitialProps && hasStaticProps) {
            throw new Error(SSG_GET_INITIAL_PROPS_CONFLICT);
        }
        if (hasGetInitialProps && hasServerProps) {
            throw new Error(SERVER_PROPS_GET_INIT_PROPS_CONFLICT);
        }
        if (hasStaticProps && hasServerProps) {
            throw new Error(SERVER_PROPS_SSG_CONFLICT);
        }
        const pageIsDynamic = isDynamicRoute(page);
        // A page cannot have static parameters if it is not a dynamic page.
        if (hasStaticProps && hasStaticPaths && !pageIsDynamic) {
            throw new Error(`getStaticPaths can only be used with dynamic pages, not '${page}'.` + `\nLearn more: https://nextjs.org/docs/routing/dynamic-routes`);
        }
        if (hasStaticProps && pageIsDynamic && !hasStaticPaths) {
            throw new Error(`getStaticPaths is required for dynamic SSG pages and is missing for '${page}'.` + `\nRead more: https://nextjs.org/docs/messages/invalid-getstaticpaths-value`);
        }
        if (hasStaticProps && hasStaticPaths || staticPathsResult) {
            ({ paths: prerenderRoutes, fallback: prerenderFallback, encodedPaths: encodedPrerenderRoutes } = await buildStaticPaths({
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
        if (config.unstable_includeFiles || config.unstable_excludeFiles) {
            Log.warn(`unstable_includeFiles/unstable_excludeFiles has been removed in favor of the option in next.config.js.\nSee more info here: https://nextjs.org/docs/advanced-features/output-file-tracing#caveats`);
        }
        let isStatic = false;
        if (!hasStaticProps && !hasGetInitialProps && !hasServerProps) {
            isStatic = true;
        }
        // When PPR is enabled, any route may be completely static, so
        // mark this route as static.
        let isPPR = false;
        if (supportsPPR) {
            isPPR = true;
            isStatic = true;
        }
        // interception routes depend on `Next-URL` and `Next-Router-State-Tree` request headers and thus cannot be prerendered
        if (isInterceptionRouteAppPath(page)) {
            isStatic = false;
            isPPR = false;
        }
        return {
            isStatic,
            isPPR,
            isHybridAmp: config.amp === "hybrid",
            isAmpOnly: config.amp === true,
            prerenderRoutes,
            prerenderFallback,
            encodedPrerenderRoutes,
            hasStaticProps,
            hasServerProps,
            isNextImageImported,
            appConfig
        };
    }).catch((err)=>{
        if (err.message === "INVALID_DEFAULT_EXPORT") {
            throw err;
        }
        console.error(err);
        throw new Error(`Failed to collect page data for ${page}`);
    });
}
export async function hasCustomGetInitialProps({ page, distDir, runtimeEnvConfig, checkingApp }) {
    require("../shared/lib/runtime-config.external").setConfig(runtimeEnvConfig);
    const components = await loadComponents({
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
export async function getDefinedNamedExports({ page, distDir, runtimeEnvConfig }) {
    require("../shared/lib/runtime-config.external").setConfig(runtimeEnvConfig);
    const components = await loadComponents({
        distDir,
        page: page,
        isAppPath: false
    });
    return Object.keys(components.ComponentMod).filter((key)=>{
        return typeof components.ComponentMod[key] !== "undefined";
    });
}
export function detectConflictingPaths(combinedPages, ssgPages, additionalSsgPaths) {
    const conflictingPaths = new Map();
    const dynamicSsgPages = [
        ...ssgPages
    ].filter((page)=>isDynamicRoute(page));
    const additionalSsgPathsByPath = {};
    additionalSsgPaths.forEach((paths, pathsPage)=>{
        additionalSsgPathsByPath[pathsPage] ||= {};
        paths.forEach((curPath)=>{
            const currentPath = curPath.toLowerCase();
            additionalSsgPathsByPath[pathsPage][currentPath] = curPath;
        });
    });
    additionalSsgPaths.forEach((paths, pathsPage)=>{
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
                    conflictingPath = additionalSsgPaths.get(page) == null ? undefined : additionalSsgPathsByPath[page][lowerPath];
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
        let conflictingPathsOutput = "";
        conflictingPaths.forEach((pathItems)=>{
            pathItems.forEach((pathItem, idx)=>{
                const isDynamic = pathItem.page !== pathItem.path;
                if (idx > 0) {
                    conflictingPathsOutput += "conflicts with ";
                }
                conflictingPathsOutput += `path: "${pathItem.path}"${isDynamic ? ` from page: "${pathItem.page}" ` : " "}`;
            });
            conflictingPathsOutput += "\n";
        });
        Log.error("Conflicting paths returned from getStaticPaths, paths must be unique per page.\n" + "See more info here: https://nextjs.org/docs/messages/conflicting-ssg-paths\n\n" + conflictingPathsOutput);
        process.exit(1);
    }
}
export async function copyTracedFiles(dir, distDir, pageKeys, appPageKeys, tracingRoot, serverConfig, middlewareManifest, hasInstrumentationHook, staticPages) {
    const outputPath = path.join(distDir, "standalone");
    let moduleType = false;
    const nextConfig = {
        ...serverConfig,
        distDir: `./${path.relative(dir, distDir)}`
    };
    try {
        const packageJsonPath = path.join(distDir, "../package.json");
        const packageJson = JSON.parse(await fs.readFile(packageJsonPath, "utf8"));
        moduleType = packageJson.type === "module";
    } catch  {}
    const copiedFiles = new Set();
    await fs.rm(outputPath, {
        recursive: true,
        force: true
    });
    async function handleTraceFiles(traceFilePath) {
        const traceData = JSON.parse(await fs.readFile(traceFilePath, "utf8"));
        const copySema = new Sema(10, {
            capacity: traceData.files.length
        });
        const traceFileDir = path.dirname(traceFilePath);
        await Promise.all(traceData.files.map(async (relativeFile)=>{
            await copySema.acquire();
            const tracedFilePath = path.join(traceFileDir, relativeFile);
            const fileOutputPath = path.join(outputPath, path.relative(tracingRoot, tracedFilePath));
            if (!copiedFiles.has(fileOutputPath)) {
                copiedFiles.add(fileOutputPath);
                await fs.mkdir(path.dirname(fileOutputPath), {
                    recursive: true
                });
                const symlink = await fs.readlink(tracedFilePath).catch(()=>null);
                if (symlink) {
                    try {
                        await fs.symlink(symlink, fileOutputPath);
                    } catch (e) {
                        if (e.code !== "EEXIST") {
                            throw e;
                        }
                    }
                } else {
                    await fs.copyFile(tracedFilePath, fileOutputPath);
                }
            }
            await copySema.release();
        }));
    }
    async function handleEdgeFunction(page) {
        var _page_wasm, _page_assets;
        async function handleFile(file) {
            const originalPath = path.join(distDir, file);
            const fileOutputPath = path.join(outputPath, path.relative(tracingRoot, distDir), file);
            await fs.mkdir(path.dirname(fileOutputPath), {
                recursive: true
            });
            await fs.copyFile(originalPath, fileOutputPath);
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
        const route = normalizePagePath(page);
        if (staticPages.has(route)) {
            continue;
        }
        const pageFile = path.join(distDir, "server", "pages", `${normalizePagePath(page)}.js`);
        const pageTraceFile = `${pageFile}.nft.json`;
        await handleTraceFiles(pageTraceFile).catch((err)=>{
            if (err.code !== "ENOENT" || page !== "/404" && page !== "/500") {
                Log.warn(`Failed to copy traced files for ${pageFile}`, err);
            }
        });
    }
    if (appPageKeys) {
        for (const page of appPageKeys){
            if (middlewareManifest.functions.hasOwnProperty(page)) {
                continue;
            }
            const pageFile = path.join(distDir, "server", "app", `${page}.js`);
            const pageTraceFile = `${pageFile}.nft.json`;
            await handleTraceFiles(pageTraceFile).catch((err)=>{
                Log.warn(`Failed to copy traced files for ${pageFile}`, err);
            });
        }
    }
    if (hasInstrumentationHook) {
        await handleTraceFiles(path.join(distDir, "server", "instrumentation.js.nft.json"));
    }
    await handleTraceFiles(path.join(distDir, "next-server.js.nft.json"));
    const serverOutputPath = path.join(outputPath, path.relative(tracingRoot, dir), "server.js");
    await fs.mkdir(path.dirname(serverOutputPath), {
        recursive: true
    });
    await fs.writeFile(serverOutputPath, `${moduleType ? `performance.mark('next-start');
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
export function isReservedPage(page) {
    return RESERVED_PAGE.test(page);
}
export function isAppBuiltinNotFoundPage(page) {
    return /next[\\/]dist[\\/]client[\\/]components[\\/]not-found-error/.test(page);
}
export function isCustomErrorPage(page) {
    return page === "/404" || page === "/500";
}
export function isMiddlewareFile(file) {
    return file === `/${MIDDLEWARE_FILENAME}` || file === `/src/${MIDDLEWARE_FILENAME}`;
}
export function isInstrumentationHookFile(file) {
    return file === `/${INSTRUMENTATION_HOOK_FILENAME}` || file === `/src/${INSTRUMENTATION_HOOK_FILENAME}`;
}
export function getPossibleInstrumentationHookFilenames(folder, extensions) {
    const files = [];
    for (const extension of extensions){
        files.push(path.join(folder, `${INSTRUMENTATION_HOOK_FILENAME}.${extension}`), path.join(folder, `src`, `${INSTRUMENTATION_HOOK_FILENAME}.${extension}`));
    }
    return files;
}
export function getPossibleMiddlewareFilenames(folder, extensions) {
    return extensions.map((extension)=>path.join(folder, `${MIDDLEWARE_FILENAME}.${extension}`));
}
export class NestedMiddlewareError extends Error {
    constructor(nestedFileNames, mainDir, pagesOrAppDir){
        super(`Nested Middleware is not allowed, found:\n` + `${nestedFileNames.map((file)=>`pages${file}`).join("\n")}\n` + `Please move your code to a single file at ${path.join(path.posix.sep, path.relative(mainDir, path.resolve(pagesOrAppDir, "..")), "middleware")} instead.\n` + `Read More - https://nextjs.org/docs/messages/nested-middleware`);
    }
}
export function getSupportedBrowsers(dir, isDevelopment) {
    let browsers;
    try {
        const browsersListConfig = browserslist.loadConfig({
            path: dir,
            env: isDevelopment ? "development" : "production"
        });
        // Running `browserslist` resolves `extends` and other config features into a list of browsers
        if (browsersListConfig && browsersListConfig.length > 0) {
            browsers = browserslist(browsersListConfig);
        }
    } catch  {}
    // When user has browserslist use that target
    if (browsers && browsers.length > 0) {
        return browsers;
    }
    // Uses modern browsers as the default.
    return MODERN_BROWSERSLIST_TARGET;
}
export function isWebpackServerOnlyLayer(layer) {
    return Boolean(layer && WEBPACK_LAYERS.GROUP.serverOnly.includes(layer));
}
export function isWebpackClientOnlyLayer(layer) {
    return Boolean(layer && WEBPACK_LAYERS.GROUP.clientOnly.includes(layer));
}
export function isWebpackDefaultLayer(layer) {
    return layer === null || layer === undefined;
}
export function isWebpackAppLayer(layer) {
    return Boolean(layer && WEBPACK_LAYERS.GROUP.app.includes(layer));
}

//# sourceMappingURL=utils.js.map