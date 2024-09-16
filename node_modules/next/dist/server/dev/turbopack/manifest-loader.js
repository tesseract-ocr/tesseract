"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "TurbopackManifestLoader", {
    enumerable: true,
    get: function() {
        return TurbopackManifestLoader;
    }
});
const _pathtoregexp = require("next/dist/compiled/path-to-regexp");
const _constants = require("../../../shared/lib/constants");
const _path = require("path");
const _promises = require("fs/promises");
const _nextjsrequirecachehotreloader = require("../../../build/webpack/plugins/nextjs-require-cache-hot-reloader");
const _writeatomic = require("../../../lib/fs/write-atomic");
const _generateinterceptionroutesrewrites = require("../../../lib/generate-interception-routes-rewrites");
const _buildmanifestplugin = require("../../../build/webpack/plugins/build-manifest-plugin");
const _getassetpathfromroute = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/router/utils/get-asset-path-from-route"));
const _entrykey = require("./entry-key");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function readPartialManifest(distDir, name, pageName, type = "pages") {
    const manifestPath = _path.posix.join(distDir, `server`, type, type === "middleware" || type === "instrumentation" ? "" : type === "app" ? pageName : (0, _getassetpathfromroute.default)(pageName), name);
    return JSON.parse(await (0, _promises.readFile)(_path.posix.join(manifestPath), "utf-8"));
}
class TurbopackManifestLoader {
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
        this.actionManifests.set((0, _entrykey.getEntryKey)("app", "server", pageName), await readPartialManifest(this.distDir, `${_constants.SERVER_REFERENCE_MANIFEST}.json`, pageName, "app"));
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
        const actionManifestJsonPath = (0, _path.join)(this.distDir, "server", `${_constants.SERVER_REFERENCE_MANIFEST}.json`);
        const actionManifestJsPath = (0, _path.join)(this.distDir, "server", `${_constants.SERVER_REFERENCE_MANIFEST}.js`);
        const json = JSON.stringify(actionManifest, null, 2);
        (0, _nextjsrequirecachehotreloader.deleteCache)(actionManifestJsonPath);
        (0, _nextjsrequirecachehotreloader.deleteCache)(actionManifestJsPath);
        await (0, _promises.writeFile)(actionManifestJsonPath, json, "utf-8");
        await (0, _promises.writeFile)(actionManifestJsPath, `self.__RSC_SERVER_MANIFEST=${JSON.stringify(json)}`, "utf-8");
    }
    async loadAppBuildManifest(pageName) {
        this.appBuildManifests.set((0, _entrykey.getEntryKey)("app", "server", pageName), await readPartialManifest(this.distDir, _constants.APP_BUILD_MANIFEST, pageName, "app"));
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
        const appBuildManifestPath = (0, _path.join)(this.distDir, _constants.APP_BUILD_MANIFEST);
        (0, _nextjsrequirecachehotreloader.deleteCache)(appBuildManifestPath);
        await (0, _writeatomic.writeFileAtomic)(appBuildManifestPath, JSON.stringify(appBuildManifest, null, 2));
    }
    async loadAppPathsManifest(pageName) {
        this.appPathsManifests.set((0, _entrykey.getEntryKey)("app", "server", pageName), await readPartialManifest(this.distDir, _constants.APP_PATHS_MANIFEST, pageName, "app"));
    }
    async writeAppPathsManifest() {
        const appPathsManifest = this.mergePagesManifests(this.appPathsManifests.values());
        const appPathsManifestPath = (0, _path.join)(this.distDir, "server", _constants.APP_PATHS_MANIFEST);
        (0, _nextjsrequirecachehotreloader.deleteCache)(appPathsManifestPath);
        await (0, _writeatomic.writeFileAtomic)(appPathsManifestPath, JSON.stringify(appPathsManifest, null, 2));
    }
    /**
   * Turbopack doesn't support this functionality, so it writes an empty manifest.
   */ async writeAutomaticFontOptimizationManifest() {
        const manifestPath = (0, _path.join)(this.distDir, "server", _constants.AUTOMATIC_FONT_OPTIMIZATION_MANIFEST);
        await (0, _writeatomic.writeFileAtomic)(manifestPath, JSON.stringify([]));
    }
    async loadBuildManifest(pageName, type = "pages") {
        this.buildManifests.set((0, _entrykey.getEntryKey)(type, "server", pageName), await readPartialManifest(this.distDir, _constants.BUILD_MANIFEST, pageName, type));
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
        const buildManifestPath = (0, _path.join)(this.distDir, _constants.BUILD_MANIFEST);
        const middlewareBuildManifestPath = (0, _path.join)(this.distDir, "server", `${_constants.MIDDLEWARE_BUILD_MANIFEST}.js`);
        const interceptionRewriteManifestPath = (0, _path.join)(this.distDir, "server", `${_constants.INTERCEPTION_ROUTE_REWRITE_MANIFEST}.js`);
        (0, _nextjsrequirecachehotreloader.deleteCache)(buildManifestPath);
        (0, _nextjsrequirecachehotreloader.deleteCache)(middlewareBuildManifestPath);
        (0, _nextjsrequirecachehotreloader.deleteCache)(interceptionRewriteManifestPath);
        await (0, _writeatomic.writeFileAtomic)(buildManifestPath, JSON.stringify(buildManifest, null, 2));
        await (0, _writeatomic.writeFileAtomic)(middlewareBuildManifestPath, `self.__BUILD_MANIFEST=${JSON.stringify(buildManifest)};`);
        const interceptionRewrites = JSON.stringify(rewrites.beforeFiles.filter(_generateinterceptionroutesrewrites.isInterceptionRouteRewrite));
        await (0, _writeatomic.writeFileAtomic)(interceptionRewriteManifestPath, `self.__INTERCEPTION_ROUTE_REWRITE_MANIFEST=${JSON.stringify(interceptionRewrites)};`);
        const content = {
            __rewrites: rewrites ? (0, _buildmanifestplugin.normalizeRewritesForBuildManifest)(rewrites) : {
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
        await (0, _writeatomic.writeFileAtomic)((0, _path.join)(this.distDir, "static", this.buildId, "_buildManifest.js"), buildManifestJs);
        await (0, _writeatomic.writeFileAtomic)((0, _path.join)(this.distDir, "static", this.buildId, "_ssgManifest.js"), _buildmanifestplugin.srcEmptySsgManifest);
    }
    async writeFallbackBuildManifest() {
        const fallbackBuildManifest = this.mergeBuildManifests([
            this.buildManifests.get((0, _entrykey.getEntryKey)("pages", "server", "_app")),
            this.buildManifests.get((0, _entrykey.getEntryKey)("pages", "server", "_error"))
        ].filter(Boolean));
        const fallbackBuildManifestPath = (0, _path.join)(this.distDir, `fallback-${_constants.BUILD_MANIFEST}`);
        (0, _nextjsrequirecachehotreloader.deleteCache)(fallbackBuildManifestPath);
        await (0, _writeatomic.writeFileAtomic)(fallbackBuildManifestPath, JSON.stringify(fallbackBuildManifest, null, 2));
    }
    async loadFontManifest(pageName, type = "pages") {
        this.fontManifests.set((0, _entrykey.getEntryKey)(type, "server", pageName), await readPartialManifest(this.distDir, `${_constants.NEXT_FONT_MANIFEST}.json`, pageName, type));
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
        const fontManifestJsonPath = (0, _path.join)(this.distDir, "server", `${_constants.NEXT_FONT_MANIFEST}.json`);
        const fontManifestJsPath = (0, _path.join)(this.distDir, "server", `${_constants.NEXT_FONT_MANIFEST}.js`);
        (0, _nextjsrequirecachehotreloader.deleteCache)(fontManifestJsonPath);
        (0, _nextjsrequirecachehotreloader.deleteCache)(fontManifestJsPath);
        await (0, _writeatomic.writeFileAtomic)(fontManifestJsonPath, json);
        await (0, _writeatomic.writeFileAtomic)(fontManifestJsPath, `self.__NEXT_FONT_MANIFEST=${JSON.stringify(json)}`);
    }
    async loadLoadableManifest(pageName, type = "pages") {
        this.loadableManifests.set((0, _entrykey.getEntryKey)(type, "server", pageName), await readPartialManifest(this.distDir, _constants.REACT_LOADABLE_MANIFEST, pageName, type));
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
        const loadableManifestPath = (0, _path.join)(this.distDir, _constants.REACT_LOADABLE_MANIFEST);
        const middlewareloadableManifestPath = (0, _path.join)(this.distDir, "server", `${_constants.MIDDLEWARE_REACT_LOADABLE_MANIFEST}.js`);
        const json = JSON.stringify(loadableManifest, null, 2);
        (0, _nextjsrequirecachehotreloader.deleteCache)(loadableManifestPath);
        (0, _nextjsrequirecachehotreloader.deleteCache)(middlewareloadableManifestPath);
        await (0, _writeatomic.writeFileAtomic)(loadableManifestPath, json);
        await (0, _writeatomic.writeFileAtomic)(middlewareloadableManifestPath, `self.__REACT_LOADABLE_MANIFEST=${JSON.stringify(json)}`);
    }
    async loadMiddlewareManifest(pageName, type) {
        this.middlewareManifests.set((0, _entrykey.getEntryKey)(type === "middleware" || type === "instrumentation" ? "root" : type, "server", pageName), await readPartialManifest(this.distDir, _constants.MIDDLEWARE_MANIFEST, pageName, type));
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
                    matcher.regexp = (0, _pathtoregexp.pathToRegexp)(matcher.originalSource, [], {
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
        const middlewareManifestPath = (0, _path.join)(this.distDir, "server", _constants.MIDDLEWARE_MANIFEST);
        (0, _nextjsrequirecachehotreloader.deleteCache)(middlewareManifestPath);
        await (0, _writeatomic.writeFileAtomic)(middlewareManifestPath, JSON.stringify(middlewareManifest, null, 2));
    }
    async loadPagesManifest(pageName) {
        this.pagesManifests.set((0, _entrykey.getEntryKey)("pages", "server", pageName), await readPartialManifest(this.distDir, _constants.PAGES_MANIFEST, pageName));
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
        const pagesManifestPath = (0, _path.join)(this.distDir, "server", _constants.PAGES_MANIFEST);
        (0, _nextjsrequirecachehotreloader.deleteCache)(pagesManifestPath);
        await (0, _writeatomic.writeFileAtomic)(pagesManifestPath, JSON.stringify(pagesManifest, null, 2));
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