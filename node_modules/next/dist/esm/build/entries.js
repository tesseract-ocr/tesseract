import { posix, join, dirname, extname } from "path";
import { stringify } from "querystring";
import fs from "fs";
import { PAGES_DIR_ALIAS, ROOT_DIR_ALIAS, APP_DIR_ALIAS, WEBPACK_LAYERS, INSTRUMENTATION_HOOK_FILENAME } from "../lib/constants";
import { isAPIRoute } from "../lib/is-api-route";
import { isEdgeRuntime } from "../lib/is-edge-runtime";
import { APP_CLIENT_INTERNALS, RSC_MODULE_TYPES, UNDERSCORE_NOT_FOUND_ROUTE_ENTRY } from "../shared/lib/constants";
import { CLIENT_STATIC_FILES_RUNTIME_AMP, CLIENT_STATIC_FILES_RUNTIME_MAIN, CLIENT_STATIC_FILES_RUNTIME_MAIN_APP, CLIENT_STATIC_FILES_RUNTIME_POLYFILLS, CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH, COMPILER_NAMES, EDGE_RUNTIME_WEBPACK } from "../shared/lib/constants";
import { isMiddlewareFile, isMiddlewareFilename, isInstrumentationHookFile, isInstrumentationHookFilename } from "./utils";
import { getPageStaticInfo } from "./analysis/get-page-static-info";
import { normalizePathSep } from "../shared/lib/page-path/normalize-path-sep";
import { normalizePagePath } from "../shared/lib/page-path/normalize-page-path";
import { normalizeAppPath } from "../shared/lib/router/utils/app-paths";
import { encodeMatchers } from "./webpack/loaders/next-middleware-loader";
import { isAppRouteRoute } from "../lib/is-app-route-route";
import { normalizeMetadataRoute } from "../lib/metadata/get-metadata-route";
import { getRouteLoaderEntry } from "./webpack/loaders/next-route-loader";
import { isInternalComponent, isNonRoutePagesPage } from "../lib/is-internal-component";
import { isStaticMetadataRouteFile } from "../lib/metadata/is-metadata-route";
import { RouteKind } from "../server/future/route-kind";
import { encodeToBase64 } from "./webpack/loaders/utils";
import { normalizeCatchAllRoutes } from "./normalize-catchall-routes";
import { PAGE_TYPES } from "../lib/page-types";
export function sortByPageExts(pageExtensions) {
    return (a, b)=>{
        // prioritize entries according to pageExtensions order
        // for consistency as fs order can differ across systems
        // NOTE: this is reversed so preferred comes last and
        // overrides prior
        const aExt = extname(a);
        const bExt = extname(b);
        const aNoExt = a.substring(0, a.length - aExt.length);
        const bNoExt = a.substring(0, b.length - bExt.length);
        if (aNoExt !== bNoExt) return 0;
        // find extension index (skip '.' as pageExtensions doesn't have it)
        const aExtIndex = pageExtensions.indexOf(aExt.substring(1));
        const bExtIndex = pageExtensions.indexOf(bExt.substring(1));
        return bExtIndex - aExtIndex;
    };
}
export async function getStaticInfoIncludingLayouts({ isInsideAppDir, pageExtensions, pageFilePath, appDir, config, isDev, page }) {
    const pageStaticInfo = await getPageStaticInfo({
        nextConfig: config,
        pageFilePath,
        isDev,
        page,
        pageType: isInsideAppDir ? PAGE_TYPES.APP : PAGE_TYPES.PAGES
    });
    const staticInfo = isInsideAppDir ? {
        // TODO-APP: Remove the rsc key altogether. It's no longer required.
        rsc: "server"
    } : pageStaticInfo;
    if (isInsideAppDir && appDir) {
        const layoutFiles = [];
        const potentialLayoutFiles = pageExtensions.map((ext)=>"layout." + ext);
        let dir = dirname(pageFilePath);
        // Uses startsWith to not include directories further up.
        while(dir.startsWith(appDir)){
            for (const potentialLayoutFile of potentialLayoutFiles){
                const layoutFile = join(dir, potentialLayoutFile);
                if (!fs.existsSync(layoutFile)) {
                    continue;
                }
                layoutFiles.unshift(layoutFile);
            }
            // Walk up the directory tree
            dir = join(dir, "..");
        }
        for (const layoutFile of layoutFiles){
            const layoutStaticInfo = await getPageStaticInfo({
                nextConfig: config,
                pageFilePath: layoutFile,
                isDev,
                page,
                pageType: isInsideAppDir ? PAGE_TYPES.APP : PAGE_TYPES.PAGES
            });
            // Only runtime is relevant here.
            if (layoutStaticInfo.runtime) {
                staticInfo.runtime = layoutStaticInfo.runtime;
            }
            if (layoutStaticInfo.preferredRegion) {
                staticInfo.preferredRegion = layoutStaticInfo.preferredRegion;
            }
        }
        if (pageStaticInfo.runtime) {
            staticInfo.runtime = pageStaticInfo.runtime;
        }
        if (pageStaticInfo.preferredRegion) {
            staticInfo.preferredRegion = pageStaticInfo.preferredRegion;
        }
        // if it's static metadata route, don't inherit runtime from layout
        const relativePath = pageFilePath.replace(appDir, "");
        if (isStaticMetadataRouteFile(relativePath)) {
            delete staticInfo.runtime;
            delete staticInfo.preferredRegion;
        }
    }
    return staticInfo;
}
/**
 * For a given page path removes the provided extensions.
 */ export function getPageFromPath(pagePath, pageExtensions) {
    let page = normalizePathSep(pagePath.replace(new RegExp(`\\.+(${pageExtensions.join("|")})$`), ""));
    page = page.replace(/\/index$/, "");
    return page === "" ? "/" : page;
}
export function getPageFilePath({ absolutePagePath, pagesDir, appDir, rootDir }) {
    if (absolutePagePath.startsWith(PAGES_DIR_ALIAS) && pagesDir) {
        return absolutePagePath.replace(PAGES_DIR_ALIAS, pagesDir);
    }
    if (absolutePagePath.startsWith(APP_DIR_ALIAS) && appDir) {
        return absolutePagePath.replace(APP_DIR_ALIAS, appDir);
    }
    if (absolutePagePath.startsWith(ROOT_DIR_ALIAS)) {
        return absolutePagePath.replace(ROOT_DIR_ALIAS, rootDir);
    }
    return require.resolve(absolutePagePath);
}
/**
 * Creates a mapping of route to page file path for a given list of page paths.
 * For example ['/middleware.ts'] is turned into  { '/middleware': `${ROOT_DIR_ALIAS}/middleware.ts` }
 */ export function createPagesMapping({ isDev, pageExtensions, pagePaths, pagesType, pagesDir }) {
    const isAppRoute = pagesType === "app";
    const pages = pagePaths.reduce((result, pagePath)=>{
        // Do not process .d.ts files as routes
        if (pagePath.endsWith(".d.ts") && pageExtensions.includes("ts")) {
            return result;
        }
        let pageKey = getPageFromPath(pagePath, pageExtensions);
        if (isAppRoute) {
            pageKey = pageKey.replace(/%5F/g, "_");
            if (pageKey === "/not-found") {
                pageKey = UNDERSCORE_NOT_FOUND_ROUTE_ENTRY;
            }
        }
        const normalizedPath = normalizePathSep(join(pagesType === "pages" ? PAGES_DIR_ALIAS : pagesType === "app" ? APP_DIR_ALIAS : ROOT_DIR_ALIAS, pagePath));
        const route = pagesType === "app" ? normalizeMetadataRoute(pageKey) : pageKey;
        result[route] = normalizedPath;
        return result;
    }, {});
    switch(pagesType){
        case PAGE_TYPES.ROOT:
            {
                return pages;
            }
        case PAGE_TYPES.APP:
            {
                const hasAppPages = Object.keys(pages).some((page)=>page.endsWith("/page"));
                return {
                    // If there's any app pages existed, add a default not-found page.
                    // If there's any custom not-found page existed, it will override the default one.
                    ...hasAppPages && {
                        [UNDERSCORE_NOT_FOUND_ROUTE_ENTRY]: "next/dist/client/components/not-found-error"
                    },
                    ...pages
                };
            }
        case PAGE_TYPES.PAGES:
            {
                if (isDev) {
                    delete pages["/_app"];
                    delete pages["/_error"];
                    delete pages["/_document"];
                }
                // In development we always alias these to allow Webpack to fallback to
                // the correct source file so that HMR can work properly when a file is
                // added or removed.
                const root = isDev && pagesDir ? PAGES_DIR_ALIAS : "next/dist/pages";
                return {
                    "/_app": `${root}/_app`,
                    "/_error": `${root}/_error`,
                    "/_document": `${root}/_document`,
                    ...pages
                };
            }
        default:
            {
                return {};
            }
    }
}
export function getEdgeServerEntry(opts) {
    var _opts_config_experimental_sri;
    if (opts.pagesType === "app" && isAppRouteRoute(opts.page) && opts.appDirLoader) {
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            appDirLoader: Buffer.from(opts.appDirLoader || "").toString("base64"),
            nextConfigOutput: opts.config.output,
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString("base64")
        };
        return {
            import: `next-edge-app-route-loader?${stringify(loaderParams)}!`,
            layer: WEBPACK_LAYERS.reactServerComponents
        };
    }
    if (isMiddlewareFile(opts.page)) {
        var _opts_middleware;
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            rootDir: opts.rootDir,
            matchers: ((_opts_middleware = opts.middleware) == null ? void 0 : _opts_middleware.matchers) ? encodeMatchers(opts.middleware.matchers) : "",
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString("base64")
        };
        return `next-middleware-loader?${stringify(loaderParams)}!`;
    }
    if (isAPIRoute(opts.page)) {
        const loaderParams = {
            absolutePagePath: opts.absolutePagePath,
            page: opts.page,
            rootDir: opts.rootDir,
            preferredRegion: opts.preferredRegion,
            middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString("base64")
        };
        return `next-edge-function-loader?${stringify(loaderParams)}!`;
    }
    const loaderParams = {
        absolute500Path: opts.pages["/500"] || "",
        absoluteAppPath: opts.pages["/_app"],
        absoluteDocumentPath: opts.pages["/_document"],
        absoluteErrorPath: opts.pages["/_error"],
        absolutePagePath: opts.absolutePagePath,
        dev: opts.isDev,
        isServerComponent: opts.isServerComponent,
        page: opts.page,
        stringifiedConfig: Buffer.from(JSON.stringify(opts.config)).toString("base64"),
        pagesType: opts.pagesType,
        appDirLoader: Buffer.from(opts.appDirLoader || "").toString("base64"),
        sriEnabled: !opts.isDev && !!((_opts_config_experimental_sri = opts.config.experimental.sri) == null ? void 0 : _opts_config_experimental_sri.algorithm),
        cacheHandler: opts.config.cacheHandler,
        preferredRegion: opts.preferredRegion,
        middlewareConfig: Buffer.from(JSON.stringify(opts.middlewareConfig || {})).toString("base64"),
        serverActions: opts.config.experimental.serverActions
    };
    return {
        import: `next-edge-ssr-loader?${JSON.stringify(loaderParams)}!`,
        // The Edge bundle includes the server in its entrypoint, so it has to
        // be in the SSR layer — we later convert the page request to the RSC layer
        // via a webpack rule.
        layer: opts.appDirLoader ? WEBPACK_LAYERS.serverSideRendering : undefined
    };
}
export function getInstrumentationEntry(opts) {
    // the '../' is needed to make sure the file is not chunked
    const filename = `${opts.isEdgeServer ? "edge-" : opts.isDev ? "" : "../"}${INSTRUMENTATION_HOOK_FILENAME}.js`;
    return {
        import: opts.absolutePagePath,
        filename,
        layer: WEBPACK_LAYERS.instrument
    };
}
export function getAppEntry(opts) {
    return {
        import: `next-app-loader?${stringify(opts)}!`,
        layer: WEBPACK_LAYERS.reactServerComponents
    };
}
export function getClientEntry(opts) {
    const loaderOptions = {
        absolutePagePath: opts.absolutePagePath,
        page: opts.page
    };
    const pageLoader = `next-client-pages-loader?${stringify(loaderOptions)}!`;
    // Make sure next/router is a dependency of _app or else chunk splitting
    // might cause the router to not be able to load causing hydration
    // to fail
    return opts.page === "/_app" ? [
        pageLoader,
        require.resolve("../client/router")
    ] : pageLoader;
}
export function runDependingOnPageType(params) {
    if (params.pageType === PAGE_TYPES.ROOT && isInstrumentationHookFile(params.page)) {
        params.onServer();
        params.onEdgeServer();
        return;
    }
    if (isMiddlewareFile(params.page)) {
        params.onEdgeServer();
        return;
    }
    if (isAPIRoute(params.page)) {
        if (isEdgeRuntime(params.pageRuntime)) {
            params.onEdgeServer();
            return;
        }
        params.onServer();
        return;
    }
    if (params.page === "/_document") {
        params.onServer();
        return;
    }
    if (params.page === "/_app" || params.page === "/_error" || params.page === "/404" || params.page === "/500") {
        params.onClient();
        params.onServer();
        return;
    }
    if (isEdgeRuntime(params.pageRuntime)) {
        params.onClient();
        params.onEdgeServer();
        return;
    }
    params.onClient();
    params.onServer();
    return;
}
export async function createEntrypoints(params) {
    const { config, pages, pagesDir, isDev, rootDir, rootPaths, appDir, appPaths, pageExtensions } = params;
    const edgeServer = {};
    const server = {};
    const client = {};
    let middlewareMatchers = undefined;
    let appPathsPerRoute = {};
    if (appDir && appPaths) {
        for(const pathname in appPaths){
            const normalizedPath = normalizeAppPath(pathname);
            const actualPath = appPaths[pathname];
            if (!appPathsPerRoute[normalizedPath]) {
                appPathsPerRoute[normalizedPath] = [];
            }
            appPathsPerRoute[normalizedPath].push(// TODO-APP: refactor to pass the page path from createPagesMapping instead.
            getPageFromPath(actualPath, pageExtensions).replace(APP_DIR_ALIAS, ""));
        }
        // TODO: find a better place to do this
        normalizeCatchAllRoutes(appPathsPerRoute);
        // Make sure to sort parallel routes to make the result deterministic.
        appPathsPerRoute = Object.fromEntries(Object.entries(appPathsPerRoute).map(([k, v])=>[
                k,
                v.sort()
            ]));
    }
    const getEntryHandler = (mappings, pagesType)=>async (page)=>{
            const bundleFile = normalizePagePath(page);
            const clientBundlePath = posix.join(pagesType, bundleFile);
            const serverBundlePath = pagesType === PAGE_TYPES.PAGES ? posix.join("pages", bundleFile) : pagesType === PAGE_TYPES.APP ? posix.join("app", bundleFile) : bundleFile.slice(1);
            const absolutePagePath = mappings[page];
            // Handle paths that have aliases
            const pageFilePath = getPageFilePath({
                absolutePagePath,
                pagesDir,
                appDir,
                rootDir
            });
            const isInsideAppDir = !!appDir && (absolutePagePath.startsWith(APP_DIR_ALIAS) || absolutePagePath.startsWith(appDir));
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
            const isServerComponent = isInsideAppDir && staticInfo.rsc !== RSC_MODULE_TYPES.client;
            if (isMiddlewareFile(page)) {
                var _staticInfo_middleware;
                middlewareMatchers = ((_staticInfo_middleware = staticInfo.middleware) == null ? void 0 : _staticInfo_middleware.matchers) ?? [
                    {
                        regexp: ".*",
                        originalSource: "/:path*"
                    }
                ];
            }
            const isInstrumentation = isInstrumentationHookFile(page) && pagesType === PAGE_TYPES.ROOT;
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
                    if (pagesType === "app" && appDir) {
                        const matchedAppPaths = appPathsPerRoute[normalizeAppPath(page)];
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
                            nextConfigExperimentalUseEarlyImport: config.experimental.useEarlyImport,
                            preferredRegion: staticInfo.preferredRegion,
                            middlewareConfig: encodeToBase64(staticInfo.middleware || {})
                        });
                    } else if (isInstrumentation) {
                        server[serverBundlePath.replace("src/", "")] = getInstrumentationEntry({
                            absolutePagePath,
                            isEdgeServer: false,
                            isDev: false
                        });
                    } else if (isAPIRoute(page)) {
                        server[serverBundlePath] = [
                            getRouteLoaderEntry({
                                kind: RouteKind.PAGES_API,
                                page,
                                absolutePagePath,
                                preferredRegion: staticInfo.preferredRegion,
                                middlewareConfig: staticInfo.middleware || {}
                            })
                        ];
                    } else if (!isMiddlewareFile(page) && !isInternalComponent(absolutePagePath) && !isNonRoutePagesPage(page)) {
                        server[serverBundlePath] = [
                            getRouteLoaderEntry({
                                kind: RouteKind.PAGES,
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
                    let appDirLoader = "";
                    if (isInstrumentation) {
                        edgeServer[serverBundlePath.replace("src/", "")] = getInstrumentationEntry({
                            absolutePagePath,
                            isEdgeServer: true,
                            isDev: false
                        });
                    } else {
                        if (pagesType === "app") {
                            const matchedAppPaths = appPathsPerRoute[normalizeAppPath(page)];
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
                                middlewareConfig: Buffer.from(JSON.stringify(staticInfo.middleware || {})).toString("base64")
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
        const entryHandler = getEntryHandler(appPaths, PAGE_TYPES.APP);
        promises.push(Promise.all(Object.keys(appPaths).map(entryHandler)));
    }
    if (rootPaths) {
        promises.push(Promise.all(Object.keys(rootPaths).map(getEntryHandler(rootPaths, PAGE_TYPES.ROOT))));
    }
    promises.push(Promise.all(Object.keys(pages).map(getEntryHandler(pages, PAGE_TYPES.PAGES))));
    await Promise.all(promises);
    return {
        client,
        server,
        edgeServer,
        middlewareMatchers
    };
}
export function finalizeEntrypoint({ name, compilerType, value, isServerComponent, hasAppDir }) {
    const entry = typeof value !== "object" || Array.isArray(value) ? {
        import: value
    } : value;
    const isApi = name.startsWith("pages/api/");
    const isInstrumentation = isInstrumentationHookFilename(name);
    switch(compilerType){
        case COMPILER_NAMES.server:
            {
                const layer = isApi ? WEBPACK_LAYERS.api : isInstrumentation ? WEBPACK_LAYERS.instrument : isServerComponent ? WEBPACK_LAYERS.reactServerComponents : undefined;
                return {
                    publicPath: isApi ? "" : undefined,
                    runtime: isApi ? "webpack-api-runtime" : "webpack-runtime",
                    layer,
                    ...entry
                };
            }
        case COMPILER_NAMES.edgeServer:
            {
                return {
                    layer: isMiddlewareFilename(name) || isApi || isInstrumentation ? WEBPACK_LAYERS.middleware : undefined,
                    library: {
                        name: [
                            "_ENTRIES",
                            `middleware_[name]`
                        ],
                        type: "assign"
                    },
                    runtime: EDGE_RUNTIME_WEBPACK,
                    asyncChunks: false,
                    ...entry
                };
            }
        case COMPILER_NAMES.client:
            {
                const isAppLayer = hasAppDir && (name === CLIENT_STATIC_FILES_RUNTIME_MAIN_APP || name === APP_CLIENT_INTERNALS || name.startsWith("app/"));
                if (// Client special cases
                name !== CLIENT_STATIC_FILES_RUNTIME_POLYFILLS && name !== CLIENT_STATIC_FILES_RUNTIME_MAIN && name !== CLIENT_STATIC_FILES_RUNTIME_MAIN_APP && name !== CLIENT_STATIC_FILES_RUNTIME_AMP && name !== CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH) {
                    if (isAppLayer) {
                        return {
                            dependOn: CLIENT_STATIC_FILES_RUNTIME_MAIN_APP,
                            layer: WEBPACK_LAYERS.appPagesBrowser,
                            ...entry
                        };
                    }
                    return {
                        dependOn: name.startsWith("pages/") && name !== "pages/_app" ? "pages/_app" : CLIENT_STATIC_FILES_RUNTIME_MAIN,
                        ...entry
                    };
                }
                if (isAppLayer) {
                    return {
                        layer: WEBPACK_LAYERS.appPagesBrowser,
                        ...entry
                    };
                }
                return entry;
            }
        default:
            {
                // Should never happen.
                throw new Error("Invalid compiler type");
            }
    }
}

//# sourceMappingURL=entries.js.map