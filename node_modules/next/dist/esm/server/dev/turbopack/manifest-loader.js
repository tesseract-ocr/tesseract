import { pathToRegexp } from "next/dist/compiled/path-to-regexp";
import { APP_BUILD_MANIFEST, APP_PATHS_MANIFEST, AUTOMATIC_FONT_OPTIMIZATION_MANIFEST, BUILD_MANIFEST, INTERCEPTION_ROUTE_REWRITE_MANIFEST, MIDDLEWARE_BUILD_MANIFEST, MIDDLEWARE_MANIFEST, MIDDLEWARE_REACT_LOADABLE_MANIFEST, NEXT_FONT_MANIFEST, PAGES_MANIFEST, REACT_LOADABLE_MANIFEST, SERVER_REFERENCE_MANIFEST } from "../../../shared/lib/constants";
import { join, posix } from "path";
import { readFile, writeFile } from "fs/promises";
import { deleteCache } from "../../../build/webpack/plugins/nextjs-require-cache-hot-reloader";
import { writeFileAtomic } from "../../../lib/fs/write-atomic";
import { isInterceptionRouteRewrite } from "../../../lib/generate-interception-routes-rewrites";
import { normalizeRewritesForBuildManifest, srcEmptySsgManifest } from "../../../build/webpack/plugins/build-manifest-plugin";
import getAssetPathFromRoute from "../../../shared/lib/router/utils/get-asset-path-from-route";
import { getEntryKey } from "./entry-key";
async function readPartialManifest(distDir, name, pageName, type = "pages") {
    const manifestPath = posix.join(distDir, `server`, type, type === "middleware" || type === "instrumentation" ? "" : type === "app" ? pageName : getAssetPathFromRoute(pageName), name);
    return JSON.parse(await readFile(posix.join(manifestPath), "utf-8"));
}
export class TurbopackManifestLoader {
    constructor({ distDir, buildId, encryptionKey }){
        this.actionManifests = new Map();
        this.appBuildManifests = new Map();
        this.appPathsManifests = new Map();
        this.buildManifests = new Map();
        this.fontManifests = new Map();
        this.loadableManifests = new Map();
        this.middlewareManifests = new Map();
        this.pagesManifests = new Map();
        this.distDir = distDir;
        this.buildId = buildId;
        this.encryptionKey = encryptionKey;
    }
    delete(key) {
        this.actionManifests.delete(key);
        this.appBuildManifests.delete(key);
        this.appPathsManifests.delete(key);
        this.buildManifests.delete(key);
        this.fontManifests.delete(key);
        this.loadableManifests.delete(key);
        this.middlewareManifests.delete(key);
        this.pagesManifests.delete(key);
    }
    async loadActionManifest(pageName) {
        this.actionManifests.set(getEntryKey("app", "server", pageName), await readPartialManifest(this.distDir, `${SERVER_REFERENCE_MANIFEST}.json`, pageName, "app"));
    }
    async mergeActionManifests(manifests) {
        const manifest = {
            node: {},
            edge: {},
            encryptionKey: this.encryptionKey
        };
        function mergeActionIds(actionEntries, other) {
            for(const key in other){
                const action = actionEntries[key] ??= {
                    workers: {},
                    layer: {}
                };
                Object.assign(action.workers, other[key].workers);
                Object.assign(action.layer, other[key].layer);
            }
        }
        for (const m of manifests){
            mergeActionIds(manifest.node, m.node);
            mergeActionIds(manifest.edge, m.edge);
        }
        return manifest;
    }
    async writeActionManifest() {
        const actionManifest = await this.mergeActionManifests(this.actionManifests.values());
        const actionManifestJsonPath = join(this.distDir, "server", `${SERVER_REFERENCE_MANIFEST}.json`);
        const actionManifestJsPath = join(this.distDir, "server", `${SERVER_REFERENCE_MANIFEST}.js`);
        const json = JSON.stringify(actionManifest, null, 2);
        deleteCache(actionManifestJsonPath);
        deleteCache(actionManifestJsPath);
        await writeFile(actionManifestJsonPath, json, "utf-8");
        await writeFile(actionManifestJsPath, `self.__RSC_SERVER_MANIFEST=${JSON.stringify(json)}`, "utf-8");
    }
    async loadAppBuildManifest(pageName) {
        this.appBuildManifests.set(getEntryKey("app", "server", pageName), await readPartialManifest(this.distDir, APP_BUILD_MANIFEST, pageName, "app"));
    }
    mergeAppBuildManifests(manifests) {
        const manifest = {
            pages: {}
        };
        for (const m of manifests){
            Object.assign(manifest.pages, m.pages);
        }
        return manifest;
    }
    async writeAppBuildManifest() {
        const appBuildManifest = this.mergeAppBuildManifests(this.appBuildManifests.values());
        const appBuildManifestPath = join(this.distDir, APP_BUILD_MANIFEST);
        deleteCache(appBuildManifestPath);
        await writeFileAtomic(appBuildManifestPath, JSON.stringify(appBuildManifest, null, 2));
    }
    async loadAppPathsManifest(pageName) {
        this.appPathsManifests.set(getEntryKey("app", "server", pageName), await readPartialManifest(this.distDir, APP_PATHS_MANIFEST, pageName, "app"));
    }
    async writeAppPathsManifest() {
        const appPathsManifest = this.mergePagesManifests(this.appPathsManifests.values());
        const appPathsManifestPath = join(this.distDir, "server", APP_PATHS_MANIFEST);
        deleteCache(appPathsManifestPath);
        await writeFileAtomic(appPathsManifestPath, JSON.stringify(appPathsManifest, null, 2));
    }
    /**
   * Turbopack doesn't support this functionality, so it writes an empty manifest.
   */ async writeAutomaticFontOptimizationManifest() {
        const manifestPath = join(this.distDir, "server", AUTOMATIC_FONT_OPTIMIZATION_MANIFEST);
        await writeFileAtomic(manifestPath, JSON.stringify([]));
    }
    async loadBuildManifest(pageName, type = "pages") {
        this.buildManifests.set(getEntryKey(type, "server", pageName), await readPartialManifest(this.distDir, BUILD_MANIFEST, pageName, type));
    }
    mergeBuildManifests(manifests) {
        const manifest = {
            pages: {
                "/_app": []
            },
            // Something in next.js depends on these to exist even for app dir rendering
            devFiles: [],
            ampDevFiles: [],
            polyfillFiles: [],
            lowPriorityFiles: [
                "static/development/_ssgManifest.js",
                "static/development/_buildManifest.js"
            ],
            rootMainFiles: [],
            ampFirstPages: []
        };
        for (const m of manifests){
            Object.assign(manifest.pages, m.pages);
            if (m.rootMainFiles.length) manifest.rootMainFiles = m.rootMainFiles;
        }
        return manifest;
    }
    async writeBuildManifest(pageEntrypoints, rewrites) {
        const buildManifest = this.mergeBuildManifests(this.buildManifests.values());
        const buildManifestPath = join(this.distDir, BUILD_MANIFEST);
        const middlewareBuildManifestPath = join(this.distDir, "server", `${MIDDLEWARE_BUILD_MANIFEST}.js`);
        const interceptionRewriteManifestPath = join(this.distDir, "server", `${INTERCEPTION_ROUTE_REWRITE_MANIFEST}.js`);
        deleteCache(buildManifestPath);
        deleteCache(middlewareBuildManifestPath);
        deleteCache(interceptionRewriteManifestPath);
        await writeFileAtomic(buildManifestPath, JSON.stringify(buildManifest, null, 2));
        await writeFileAtomic(middlewareBuildManifestPath, `self.__BUILD_MANIFEST=${JSON.stringify(buildManifest)};`);
        const interceptionRewrites = JSON.stringify(rewrites.beforeFiles.filter(isInterceptionRouteRewrite));
        await writeFileAtomic(interceptionRewriteManifestPath, `self.__INTERCEPTION_ROUTE_REWRITE_MANIFEST=${JSON.stringify(interceptionRewrites)};`);
        const content = {
            __rewrites: rewrites ? normalizeRewritesForBuildManifest(rewrites) : {
                afterFiles: [],
                beforeFiles: [],
                fallback: []
            },
            ...Object.fromEntries([
                ...pageEntrypoints.keys()
            ].map((pathname)=>[
                    pathname,
                    `static/chunks/pages${pathname === "/" ? "/index" : pathname}.js`
                ])),
            sortedPages: [
                ...pageEntrypoints.keys()
            ]
        };
        const buildManifestJs = `self.__BUILD_MANIFEST = ${JSON.stringify(content)};self.__BUILD_MANIFEST_CB && self.__BUILD_MANIFEST_CB()`;
        await writeFileAtomic(join(this.distDir, "static", this.buildId, "_buildManifest.js"), buildManifestJs);
        await writeFileAtomic(join(this.distDir, "static", this.buildId, "_ssgManifest.js"), srcEmptySsgManifest);
    }
    async writeFallbackBuildManifest() {
        const fallbackBuildManifest = this.mergeBuildManifests([
            this.buildManifests.get(getEntryKey("pages", "server", "_app")),
            this.buildManifests.get(getEntryKey("pages", "server", "_error"))
        ].filter(Boolean));
        const fallbackBuildManifestPath = join(this.distDir, `fallback-${BUILD_MANIFEST}`);
        deleteCache(fallbackBuildManifestPath);
        await writeFileAtomic(fallbackBuildManifestPath, JSON.stringify(fallbackBuildManifest, null, 2));
    }
    async loadFontManifest(pageName, type = "pages") {
        this.fontManifests.set(getEntryKey(type, "server", pageName), await readPartialManifest(this.distDir, `${NEXT_FONT_MANIFEST}.json`, pageName, type));
    }
    mergeFontManifests(manifests) {
        const manifest = {
            app: {},
            appUsingSizeAdjust: false,
            pages: {},
            pagesUsingSizeAdjust: false
        };
        for (const m of manifests){
            Object.assign(manifest.app, m.app);
            Object.assign(manifest.pages, m.pages);
            manifest.appUsingSizeAdjust = manifest.appUsingSizeAdjust || m.appUsingSizeAdjust;
            manifest.pagesUsingSizeAdjust = manifest.pagesUsingSizeAdjust || m.pagesUsingSizeAdjust;
        }
        return manifest;
    }
    async writeNextFontManifest() {
        const fontManifest = this.mergeFontManifests(this.fontManifests.values());
        const json = JSON.stringify(fontManifest, null, 2);
        const fontManifestJsonPath = join(this.distDir, "server", `${NEXT_FONT_MANIFEST}.json`);
        const fontManifestJsPath = join(this.distDir, "server", `${NEXT_FONT_MANIFEST}.js`);
        deleteCache(fontManifestJsonPath);
        deleteCache(fontManifestJsPath);
        await writeFileAtomic(fontManifestJsonPath, json);
        await writeFileAtomic(fontManifestJsPath, `self.__NEXT_FONT_MANIFEST=${JSON.stringify(json)}`);
    }
    async loadLoadableManifest(pageName, type = "pages") {
        this.loadableManifests.set(getEntryKey(type, "server", pageName), await readPartialManifest(this.distDir, REACT_LOADABLE_MANIFEST, pageName, type));
    }
    mergeLoadableManifests(manifests) {
        const manifest = {};
        for (const m of manifests){
            Object.assign(manifest, m);
        }
        return manifest;
    }
    async writeLoadableManifest() {
        const loadableManifest = this.mergeLoadableManifests(this.loadableManifests.values());
        const loadableManifestPath = join(this.distDir, REACT_LOADABLE_MANIFEST);
        const middlewareloadableManifestPath = join(this.distDir, "server", `${MIDDLEWARE_REACT_LOADABLE_MANIFEST}.js`);
        const json = JSON.stringify(loadableManifest, null, 2);
        deleteCache(loadableManifestPath);
        deleteCache(middlewareloadableManifestPath);
        await writeFileAtomic(loadableManifestPath, json);
        await writeFileAtomic(middlewareloadableManifestPath, `self.__REACT_LOADABLE_MANIFEST=${JSON.stringify(json)}`);
    }
    async loadMiddlewareManifest(pageName, type) {
        this.middlewareManifests.set(getEntryKey(type === "middleware" || type === "instrumentation" ? "root" : type, "server", pageName), await readPartialManifest(this.distDir, MIDDLEWARE_MANIFEST, pageName, type));
    }
    getMiddlewareManifest(key) {
        return this.middlewareManifests.get(key);
    }
    deleteMiddlewareManifest(key) {
        return this.middlewareManifests.delete(key);
    }
    mergeMiddlewareManifests(manifests) {
        const manifest = {
            version: 3,
            middleware: {},
            sortedMiddleware: [],
            functions: {}
        };
        let instrumentation = undefined;
        for (const m of manifests){
            Object.assign(manifest.functions, m.functions);
            Object.assign(manifest.middleware, m.middleware);
            if (m.instrumentation) {
                instrumentation = m.instrumentation;
            }
        }
        const updateFunctionDefinition = (fun)=>{
            return {
                ...fun,
                files: [
                    ...(instrumentation == null ? void 0 : instrumentation.files) ?? [],
                    ...fun.files
                ]
            };
        };
        for (const key of Object.keys(manifest.middleware)){
            const value = manifest.middleware[key];
            manifest.middleware[key] = updateFunctionDefinition(value);
        }
        for (const key of Object.keys(manifest.functions)){
            const value = manifest.functions[key];
            manifest.functions[key] = updateFunctionDefinition(value);
        }
        for (const fun of Object.values(manifest.functions).concat(Object.values(manifest.middleware))){
            for (const matcher of fun.matchers){
                if (!matcher.regexp) {
                    matcher.regexp = pathToRegexp(matcher.originalSource, [], {
                        delimiter: "/",
                        sensitive: false,
                        strict: true
                    }).source.replaceAll("\\/", "/");
                }
            }
        }
        manifest.sortedMiddleware = Object.keys(manifest.middleware);
        return manifest;
    }
    async writeMiddlewareManifest() {
        const middlewareManifest = this.mergeMiddlewareManifests(this.middlewareManifests.values());
        const middlewareManifestPath = join(this.distDir, "server", MIDDLEWARE_MANIFEST);
        deleteCache(middlewareManifestPath);
        await writeFileAtomic(middlewareManifestPath, JSON.stringify(middlewareManifest, null, 2));
    }
    async loadPagesManifest(pageName) {
        this.pagesManifests.set(getEntryKey("pages", "server", pageName), await readPartialManifest(this.distDir, PAGES_MANIFEST, pageName));
    }
    mergePagesManifests(manifests) {
        const manifest = {};
        for (const m of manifests){
            Object.assign(manifest, m);
        }
        return manifest;
    }
    async writePagesManifest() {
        const pagesManifest = this.mergePagesManifests(this.pagesManifests.values());
        const pagesManifestPath = join(this.distDir, "server", PAGES_MANIFEST);
        deleteCache(pagesManifestPath);
        await writeFileAtomic(pagesManifestPath, JSON.stringify(pagesManifest, null, 2));
    }
    async writeManifests({ rewrites, pageEntrypoints }) {
        await this.writeActionManifest();
        await this.writeAppBuildManifest();
        await this.writeAppPathsManifest();
        await this.writeAutomaticFontOptimizationManifest();
        await this.writeBuildManifest(pageEntrypoints, rewrites);
        await this.writeFallbackBuildManifest();
        await this.writeLoadableManifest();
        await this.writeMiddlewareManifest();
        await this.writeNextFontManifest();
        await this.writePagesManifest();
    }
}

//# sourceMappingURL=manifest-loader.js.map