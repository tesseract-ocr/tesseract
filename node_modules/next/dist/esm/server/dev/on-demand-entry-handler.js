import createDebug from "next/dist/compiled/debug";
import { EventEmitter } from "events";
import { findPageFile } from "../lib/find-page-file";
import { getStaticInfoIncludingLayouts, runDependingOnPageType } from "../../build/entries";
import { join, posix } from "path";
import { normalizePathSep } from "../../shared/lib/page-path/normalize-path-sep";
import { normalizePagePath } from "../../shared/lib/page-path/normalize-page-path";
import { ensureLeadingSlash } from "../../shared/lib/page-path/ensure-leading-slash";
import { removePagePathTail } from "../../shared/lib/page-path/remove-page-path-tail";
import { reportTrigger } from "../../build/output";
import getRouteFromEntrypoint from "../get-route-from-entrypoint";
import { isInstrumentationHookFile, isInstrumentationHookFilename, isMiddlewareFile, isMiddlewareFilename } from "../../build/utils";
import { PageNotFoundError, stringifyError } from "../../shared/lib/utils";
import { COMPILER_INDEXES, COMPILER_NAMES, RSC_MODULE_TYPES, UNDERSCORE_NOT_FOUND_ROUTE_ENTRY } from "../../shared/lib/constants";
import { PAGE_SEGMENT_KEY } from "../../shared/lib/segment";
import { HMR_ACTIONS_SENT_TO_BROWSER } from "./hot-reloader-types";
import { isAppPageRouteDefinition } from "../future/route-definitions/app-page-route-definition";
import { scheduleOnNextTick } from "../../lib/scheduler";
import { Batcher } from "../../lib/batcher";
import { normalizeAppPath } from "../../shared/lib/router/utils/app-paths";
import { PAGE_TYPES } from "../../lib/page-types";
const debug = createDebug("next:on-demand-entry-handler");
/**
 * Returns object keys with type inferred from the object key
 */ const keys = Object.keys;
const COMPILER_KEYS = keys(COMPILER_INDEXES);
function treePathToEntrypoint(segmentPath, parentPath) {
    const [parallelRouteKey, segment] = segmentPath;
    // TODO-APP: modify this path to cover parallelRouteKey convention
    const path = (parentPath ? parentPath + "/" : "") + (parallelRouteKey !== "children" && !segment.startsWith("@") ? `@${parallelRouteKey}/` : "") + (segment === "" ? "page" : segment);
    // Last segment
    if (segmentPath.length === 2) {
        return path;
    }
    const childSegmentPath = segmentPath.slice(2);
    return treePathToEntrypoint(childSegmentPath, path);
}
function convertDynamicParamTypeToSyntax(dynamicParamTypeShort, param) {
    switch(dynamicParamTypeShort){
        case "c":
        case "ci":
            return `[...${param}]`;
        case "oc":
            return `[[...${param}]]`;
        case "d":
        case "di":
            return `[${param}]`;
        default:
            throw new Error("Unknown dynamic param type");
    }
}
/**
 * format: {compiler type}@{page type}@{page path}
 * e.g. client@pages@/index
 * e.g. server@app@app/page
 *
 * This guarantees the uniqueness for each page, to avoid conflicts between app/ and pages/
 */ export function getEntryKey(compilerType, pageBundleType, page) {
    // TODO: handle the /children slot better
    // this is a quick hack to handle when children is provided as children/page instead of /page
    const pageKey = page.replace(/(@[^/]+)\/children/g, "$1");
    return `${compilerType}@${pageBundleType}@${pageKey}`;
}
function getPageBundleType(pageBundlePath) {
    // Handle special case for /_error
    if (pageBundlePath === "/_error") return PAGE_TYPES.PAGES;
    if (isMiddlewareFilename(pageBundlePath)) return PAGE_TYPES.ROOT;
    return pageBundlePath.startsWith("pages/") ? PAGE_TYPES.PAGES : pageBundlePath.startsWith("app/") ? PAGE_TYPES.APP : PAGE_TYPES.ROOT;
}
function getEntrypointsFromTree(tree, isFirst, parentPath = []) {
    const [segment, parallelRoutes] = tree;
    const currentSegment = Array.isArray(segment) ? convertDynamicParamTypeToSyntax(segment[2], segment[0]) : segment;
    const isPageSegment = currentSegment.startsWith(PAGE_SEGMENT_KEY);
    const currentPath = [
        ...parentPath,
        isPageSegment ? "" : currentSegment
    ];
    if (!isFirst && isPageSegment) {
        // TODO get rid of '' at the start of tree
        return [
            treePathToEntrypoint(currentPath.slice(1))
        ];
    }
    return Object.keys(parallelRoutes).reduce((paths, key)=>{
        const childTree = parallelRoutes[key];
        const childPages = getEntrypointsFromTree(childTree, false, [
            ...currentPath,
            key
        ]);
        return [
            ...paths,
            ...childPages
        ];
    }, []);
}
export const ADDED = Symbol("added");
export const BUILDING = Symbol("building");
export const BUILT = Symbol("built");
export var EntryTypes;
(function(EntryTypes) {
    EntryTypes[EntryTypes["ENTRY"] = 0] = "ENTRY";
    EntryTypes[EntryTypes["CHILD_ENTRY"] = 1] = "CHILD_ENTRY";
})(EntryTypes || (EntryTypes = {}));
const entriesMap = new Map();
// remove /server from end of output for server compiler
const normalizeOutputPath = (dir)=>dir.replace(/[/\\]server$/, "");
export const getEntries = (dir)=>{
    dir = normalizeOutputPath(dir);
    const entries = entriesMap.get(dir) || {};
    entriesMap.set(dir, entries);
    return entries;
};
const invalidators = new Map();
export const getInvalidator = (dir)=>{
    dir = normalizeOutputPath(dir);
    return invalidators.get(dir);
};
const doneCallbacks = new EventEmitter();
const lastClientAccessPages = [
    ""
];
const lastServerAccessPagesForAppDir = [
    ""
];
// Make sure only one invalidation happens at a time
// Otherwise, webpack hash gets changed and it'll force the client to reload.
class Invalidator {
    constructor(multiCompiler){
        this.building = new Set();
        this.rebuildAgain = new Set();
        this.multiCompiler = multiCompiler;
    }
    shouldRebuildAll() {
        return this.rebuildAgain.size > 0;
    }
    invalidate(compilerKeys = COMPILER_KEYS) {
        for (const key of compilerKeys){
            var _this_multiCompiler_compilers_COMPILER_INDEXES_key_watching;
            // If there's a current build is processing, we won't abort it by invalidating.
            // (If aborted, it'll cause a client side hard reload)
            // But let it to invalidate just after the completion.
            // So, it can re-build the queued pages at once.
            if (this.building.has(key)) {
                this.rebuildAgain.add(key);
                continue;
            }
            this.building.add(key);
            (_this_multiCompiler_compilers_COMPILER_INDEXES_key_watching = this.multiCompiler.compilers[COMPILER_INDEXES[key]].watching) == null ? void 0 : _this_multiCompiler_compilers_COMPILER_INDEXES_key_watching.invalidate();
        }
    }
    startBuilding(compilerKey) {
        this.building.add(compilerKey);
    }
    doneBuilding(compilerKeys = []) {
        const rebuild = [];
        for (const key of compilerKeys){
            this.building.delete(key);
            if (this.rebuildAgain.has(key)) {
                rebuild.push(key);
                this.rebuildAgain.delete(key);
            }
        }
        this.invalidate(rebuild);
    }
    willRebuild(compilerKey) {
        return this.rebuildAgain.has(compilerKey);
    }
}
function disposeInactiveEntries(entries, maxInactiveAge) {
    Object.keys(entries).forEach((entryKey)=>{
        const entryData = entries[entryKey];
        const { lastActiveTime, status, dispose, bundlePath } = entryData;
        // TODO-APP: implement disposing of CHILD_ENTRY
        if (entryData.type === 1) {
            return;
        }
        // For the root middleware and the instrumentation hook files,
        // we don't dispose them periodically as it's needed for every request.
        if (isMiddlewareFilename(bundlePath) || isInstrumentationHookFilename(bundlePath)) {
            return;
        }
        if (dispose) // Skip pages already scheduled for disposing
        return;
        // This means this entry is currently building or just added
        // We don't need to dispose those entries.
        if (status !== BUILT) return;
        // We should not build the last accessed page even we didn't get any pings
        // Sometimes, it's possible our XHR ping to wait before completing other requests.
        // In that case, we should not dispose the current viewing page
        if (lastClientAccessPages.includes(entryKey) || lastServerAccessPagesForAppDir.includes(entryKey)) return;
        if (lastActiveTime && Date.now() - lastActiveTime > maxInactiveAge) {
            entries[entryKey].dispose = true;
        }
    });
}
// Normalize both app paths and page paths
function tryToNormalizePagePath(page) {
    try {
        return normalizePagePath(page);
    } catch (err) {
        console.error(err);
        throw new PageNotFoundError(page);
    }
}
/**
 * Attempts to find a page file path from the given pages absolute directory,
 * a page and allowed extensions. If the page can't be found it will throw an
 * error. It defaults the `/_error` page to Next.js internal error page.
 *
 * @param rootDir Absolute path to the project root.
 * @param page The page normalized (it will be denormalized).
 * @param extensions Array of page extensions.
 * @param pagesDir Absolute path to the pages folder with trailing `/pages`.
 * @param appDir Absolute path to the app folder with trailing `/app`.
 */ export async function findPagePathData(rootDir, page, extensions, pagesDir, appDir) {
    const normalizedPagePath = tryToNormalizePagePath(page);
    let pagePath = null;
    const isInstrumentation = isInstrumentationHookFile(normalizedPagePath);
    if (isMiddlewareFile(normalizedPagePath) || isInstrumentation) {
        pagePath = await findPageFile(rootDir, normalizedPagePath, extensions, false);
        if (!pagePath) {
            throw new PageNotFoundError(normalizedPagePath);
        }
        const pageUrl = ensureLeadingSlash(removePagePathTail(normalizePathSep(pagePath), {
            extensions
        }));
        let bundlePath = normalizedPagePath;
        let pageKey = posix.normalize(pageUrl);
        if (isInstrumentation) {
            bundlePath = bundlePath.replace("/src", "");
            pageKey = page.replace("/src", "");
        }
        return {
            filename: join(rootDir, pagePath),
            bundlePath: bundlePath.slice(1),
            page: pageKey
        };
    }
    // Check appDir first falling back to pagesDir
    if (appDir) {
        if (page === UNDERSCORE_NOT_FOUND_ROUTE_ENTRY) {
            const notFoundPath = await findPageFile(appDir, "not-found", extensions, true);
            if (notFoundPath) {
                return {
                    filename: join(appDir, notFoundPath),
                    bundlePath: `app${UNDERSCORE_NOT_FOUND_ROUTE_ENTRY}`,
                    page: UNDERSCORE_NOT_FOUND_ROUTE_ENTRY
                };
            }
            return {
                filename: require.resolve("next/dist/client/components/not-found-error"),
                bundlePath: `app${UNDERSCORE_NOT_FOUND_ROUTE_ENTRY}`,
                page: UNDERSCORE_NOT_FOUND_ROUTE_ENTRY
            };
        }
        pagePath = await findPageFile(appDir, normalizedPagePath, extensions, true);
        if (pagePath) {
            const pageUrl = ensureLeadingSlash(removePagePathTail(normalizePathSep(pagePath), {
                keepIndex: true,
                extensions
            }));
            return {
                filename: join(appDir, pagePath),
                bundlePath: posix.join("app", pageUrl),
                page: posix.normalize(pageUrl)
            };
        }
    }
    if (!pagePath && pagesDir) {
        pagePath = await findPageFile(pagesDir, normalizedPagePath, extensions, false);
    }
    if (pagePath !== null && pagesDir) {
        const pageUrl = ensureLeadingSlash(removePagePathTail(normalizePathSep(pagePath), {
            extensions
        }));
        return {
            filename: join(pagesDir, pagePath),
            bundlePath: posix.join("pages", normalizePagePath(pageUrl)),
            page: posix.normalize(pageUrl)
        };
    }
    if (page === "/_error") {
        return {
            filename: require.resolve("next/dist/pages/_error"),
            bundlePath: page,
            page: normalizePathSep(page)
        };
    } else {
        throw new PageNotFoundError(normalizedPagePath);
    }
}
export function onDemandEntryHandler({ hotReloader, maxInactiveAge, multiCompiler, nextConfig, pagesBufferLength, pagesDir, rootDir, appDir }) {
    const hasAppDir = !!appDir;
    let curInvalidator = getInvalidator(multiCompiler.outputPath);
    const curEntries = getEntries(multiCompiler.outputPath);
    if (!curInvalidator) {
        curInvalidator = new Invalidator(multiCompiler);
        invalidators.set(multiCompiler.outputPath, curInvalidator);
    }
    const startBuilding = (compilation)=>{
        const compilationName = compilation.name;
        curInvalidator.startBuilding(compilationName);
    };
    for (const compiler of multiCompiler.compilers){
        compiler.hooks.make.tap("NextJsOnDemandEntries", startBuilding);
    }
    function getPagePathsFromEntrypoints(type, entrypoints) {
        const pagePaths = [];
        for (const entrypoint of entrypoints.values()){
            const page = getRouteFromEntrypoint(entrypoint.name, hasAppDir);
            if (page) {
                var _entrypoint_name;
                const pageBundleType = ((_entrypoint_name = entrypoint.name) == null ? void 0 : _entrypoint_name.startsWith("app/")) ? PAGE_TYPES.APP : PAGE_TYPES.PAGES;
                pagePaths.push(getEntryKey(type, pageBundleType, page));
            } else if (isMiddlewareFilename(entrypoint.name) || isInstrumentationHookFilename(entrypoint.name)) {
                pagePaths.push(getEntryKey(type, PAGE_TYPES.ROOT, `/${entrypoint.name}`));
            }
        }
        return pagePaths;
    }
    for (const compiler of multiCompiler.compilers){
        compiler.hooks.done.tap("NextJsOnDemandEntries", ()=>{
            var _getInvalidator;
            return (_getInvalidator = getInvalidator(compiler.outputPath)) == null ? void 0 : _getInvalidator.doneBuilding([
                compiler.name
            ]);
        });
    }
    multiCompiler.hooks.done.tap("NextJsOnDemandEntries", (multiStats)=>{
        var _getInvalidator;
        const [clientStats, serverStats, edgeServerStats] = multiStats.stats;
        const entryNames = [
            ...getPagePathsFromEntrypoints(COMPILER_NAMES.client, clientStats.compilation.entrypoints),
            ...getPagePathsFromEntrypoints(COMPILER_NAMES.server, serverStats.compilation.entrypoints),
            ...edgeServerStats ? getPagePathsFromEntrypoints(COMPILER_NAMES.edgeServer, edgeServerStats.compilation.entrypoints) : []
        ];
        for (const name of entryNames){
            const entry = curEntries[name];
            if (!entry) {
                continue;
            }
            if (entry.status !== BUILDING) {
                continue;
            }
            entry.status = BUILT;
            doneCallbacks.emit(name);
        }
        (_getInvalidator = getInvalidator(multiCompiler.outputPath)) == null ? void 0 : _getInvalidator.doneBuilding([
            ...COMPILER_KEYS
        ]);
    });
    const pingIntervalTime = Math.max(1000, Math.min(5000, maxInactiveAge));
    setInterval(function() {
        disposeInactiveEntries(curEntries, maxInactiveAge);
    }, pingIntervalTime + 1000).unref();
    function handleAppDirPing(tree) {
        const pages = getEntrypointsFromTree(tree, true);
        for (const page of pages){
            for (const compilerType of [
                COMPILER_NAMES.client,
                COMPILER_NAMES.server,
                COMPILER_NAMES.edgeServer
            ]){
                const entryKey = getEntryKey(compilerType, PAGE_TYPES.APP, `/${page}`);
                const entryInfo = curEntries[entryKey];
                // If there's no entry, it may have been invalidated and needs to be re-built.
                if (!entryInfo) {
                    continue;
                }
                // We don't need to maintain active state of anything other than BUILT entries
                if (entryInfo.status !== BUILT) continue;
                // If there's an entryInfo
                if (!lastServerAccessPagesForAppDir.includes(entryKey)) {
                    lastServerAccessPagesForAppDir.unshift(entryKey);
                    // Maintain the buffer max length
                    // TODO: verify that the current pageKey is not at the end of the array as multiple entrypoints can exist
                    if (lastServerAccessPagesForAppDir.length > pagesBufferLength) {
                        lastServerAccessPagesForAppDir.pop();
                    }
                }
                entryInfo.lastActiveTime = Date.now();
                entryInfo.dispose = false;
            }
        }
    }
    function handlePing(pg) {
        const page = normalizePathSep(pg);
        for (const compilerType of [
            COMPILER_NAMES.client,
            COMPILER_NAMES.server,
            COMPILER_NAMES.edgeServer
        ]){
            const entryKey = getEntryKey(compilerType, PAGE_TYPES.PAGES, page);
            const entryInfo = curEntries[entryKey];
            // If there's no entry, it may have been invalidated and needs to be re-built.
            if (!entryInfo) {
                // if (page !== lastEntry) client pings, but there's no entry for page
                if (compilerType === COMPILER_NAMES.client) {
                    return;
                }
                continue;
            }
            // We don't need to maintain active state of anything other than BUILT entries
            if (entryInfo.status !== BUILT) continue;
            // If there's an entryInfo
            if (!lastClientAccessPages.includes(entryKey)) {
                lastClientAccessPages.unshift(entryKey);
                // Maintain the buffer max length
                if (lastClientAccessPages.length > pagesBufferLength) {
                    lastClientAccessPages.pop();
                }
            }
            entryInfo.lastActiveTime = Date.now();
            entryInfo.dispose = false;
        }
        return;
    }
    async function ensurePageImpl({ page, appPaths, definition, isApp, url }) {
        const stalledTime = 60;
        const stalledEnsureTimeout = setTimeout(()=>{
            debug(`Ensuring ${page} has taken longer than ${stalledTime}s, if this continues to stall this may be a bug`);
        }, stalledTime * 1000);
        try {
            let route;
            if (definition) {
                route = definition;
            } else {
                route = await findPagePathData(rootDir, page, nextConfig.pageExtensions, pagesDir, appDir);
            }
            const isInsideAppDir = !!appDir && route.filename.startsWith(appDir);
            if (typeof isApp === "boolean" && isApp !== isInsideAppDir) {
                Error.stackTraceLimit = 15;
                throw new Error(`Ensure bailed, found path "${route.page}" does not match ensure type (${isApp ? "app" : "pages"})`);
            }
            const pageBundleType = getPageBundleType(route.bundlePath);
            const addEntry = (compilerType)=>{
                const entryKey = getEntryKey(compilerType, pageBundleType, route.page);
                if (curEntries[entryKey] && // there can be an overlap in the entryKey for the instrumentation hook file and a page named the same
                // this is a quick fix to support this scenario by overwriting the instrumentation hook entry, since we only use it one time
                // any changes to the instrumentation hook file will require a restart of the dev server anyway
                !isInstrumentationHookFilename(curEntries[entryKey].bundlePath)) {
                    curEntries[entryKey].dispose = false;
                    curEntries[entryKey].lastActiveTime = Date.now();
                    if (curEntries[entryKey].status === BUILT) {
                        return {
                            entryKey,
                            newEntry: false,
                            shouldInvalidate: false
                        };
                    }
                    return {
                        entryKey,
                        newEntry: false,
                        shouldInvalidate: true
                    };
                }
                curEntries[entryKey] = {
                    type: 0,
                    appPaths,
                    absolutePagePath: route.filename,
                    request: route.filename,
                    bundlePath: route.bundlePath,
                    dispose: false,
                    lastActiveTime: Date.now(),
                    status: ADDED
                };
                return {
                    entryKey: entryKey,
                    newEntry: true,
                    shouldInvalidate: true
                };
            };
            const staticInfo = await getStaticInfoIncludingLayouts({
                page,
                pageFilePath: route.filename,
                isInsideAppDir,
                pageExtensions: nextConfig.pageExtensions,
                isDev: true,
                config: nextConfig,
                appDir
            });
            const added = new Map();
            const isServerComponent = isInsideAppDir && staticInfo.rsc !== RSC_MODULE_TYPES.client;
            runDependingOnPageType({
                page: route.page,
                pageRuntime: staticInfo.runtime,
                pageType: pageBundleType,
                onClient: ()=>{
                    // Skip adding the client entry for app / Server Components.
                    if (isServerComponent || isInsideAppDir) {
                        return;
                    }
                    added.set(COMPILER_NAMES.client, addEntry(COMPILER_NAMES.client));
                },
                onServer: ()=>{
                    added.set(COMPILER_NAMES.server, addEntry(COMPILER_NAMES.server));
                    const edgeServerEntry = getEntryKey(COMPILER_NAMES.edgeServer, pageBundleType, route.page);
                    if (curEntries[edgeServerEntry] && !isInstrumentationHookFile(route.page)) {
                        // Runtime switched from edge to server
                        delete curEntries[edgeServerEntry];
                    }
                },
                onEdgeServer: ()=>{
                    added.set(COMPILER_NAMES.edgeServer, addEntry(COMPILER_NAMES.edgeServer));
                    const serverEntry = getEntryKey(COMPILER_NAMES.server, pageBundleType, route.page);
                    if (curEntries[serverEntry] && !isInstrumentationHookFile(route.page)) {
                        // Runtime switched from server to edge
                        delete curEntries[serverEntry];
                    }
                }
            });
            const addedValues = [
                ...added.values()
            ];
            const entriesThatShouldBeInvalidated = [
                ...added.entries()
            ].filter(([, entry])=>entry.shouldInvalidate);
            const hasNewEntry = addedValues.some((entry)=>entry.newEntry);
            if (hasNewEntry) {
                const routePage = isApp ? route.page : normalizeAppPath(route.page);
                reportTrigger(routePage, url);
            }
            if (entriesThatShouldBeInvalidated.length > 0) {
                const invalidatePromise = Promise.all(entriesThatShouldBeInvalidated.map(([compilerKey, { entryKey }])=>{
                    return new Promise((resolve, reject)=>{
                        doneCallbacks.once(entryKey, (err)=>{
                            if (err) {
                                return reject(err);
                            }
                            // If the invalidation also triggers a rebuild, we need to
                            // wait for that additional build to prevent race conditions.
                            const needsRebuild = curInvalidator.willRebuild(compilerKey);
                            if (needsRebuild) {
                                doneCallbacks.once(entryKey, (rebuildErr)=>{
                                    if (rebuildErr) {
                                        return reject(rebuildErr);
                                    }
                                    resolve();
                                });
                            } else {
                                resolve();
                            }
                        });
                    });
                }));
                curInvalidator.invalidate([
                    ...added.keys()
                ]);
                await invalidatePromise;
            }
        } finally{
            clearTimeout(stalledEnsureTimeout);
        }
    }
    // Make sure that we won't have multiple invalidations ongoing concurrently.
    const batcher = Batcher.create({
        // The cache key here is composed of the elements that affect the
        // compilation, namely, the page, whether it's client only, and whether
        // it's an app page. This ensures that we don't have multiple compilations
        // for the same page happening concurrently.
        //
        // We don't include the whole match because it contains match specific
        // parameters (like route params) that would just bust this cache. Any
        // details that would possibly bust the cache should be listed here.
        cacheKeyFn: (options)=>JSON.stringify(options),
        // Schedule the invocation of the ensurePageImpl function on the next tick.
        schedulerFn: scheduleOnNextTick
    });
    return {
        async ensurePage ({ page, appPaths = null, definition, isApp, url }) {
            // If the route is actually an app page route, then we should have access
            // to the app route definition, and therefore, the appPaths from it.
            if (!appPaths && definition && isAppPageRouteDefinition(definition)) {
                appPaths = definition.appPaths;
            }
            // Wrap the invocation of the ensurePageImpl function in the pending
            // wrapper, which will ensure that we don't have multiple compilations
            // for the same page happening concurrently.
            return batcher.batch({
                page,
                appPaths,
                definition,
                isApp
            }, async ()=>{
                await ensurePageImpl({
                    page,
                    appPaths,
                    definition,
                    isApp,
                    url
                });
            });
        },
        onHMR (client, getHmrServerError) {
            let bufferedHmrServerError = null;
            client.addEventListener("close", ()=>{
                bufferedHmrServerError = null;
            });
            client.addEventListener("message", ({ data })=>{
                try {
                    const error = getHmrServerError();
                    // New error occurred: buffered error is flushed and new error occurred
                    if (!bufferedHmrServerError && error) {
                        hotReloader.send({
                            action: HMR_ACTIONS_SENT_TO_BROWSER.SERVER_ERROR,
                            errorJSON: stringifyError(error)
                        });
                        bufferedHmrServerError = null;
                    }
                    const parsedData = JSON.parse(typeof data !== "string" ? data.toString() : data);
                    if (parsedData.event === "ping") {
                        if (parsedData.appDirRoute) {
                            handleAppDirPing(parsedData.tree);
                        } else {
                            handlePing(parsedData.page);
                        }
                    }
                } catch  {}
            });
        }
    };
}

//# sourceMappingURL=on-demand-entry-handler.js.map