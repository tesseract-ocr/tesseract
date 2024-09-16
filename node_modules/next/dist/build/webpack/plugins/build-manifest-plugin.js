"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    getEntrypointFiles: null,
    normalizeRewritesForBuildManifest: null,
    srcEmptySsgManifest: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    // This plugin creates a build-manifest.json for all assets that are being output
    // It has a mapping of "entry" filename to real filename. Because the real filename can be hashed in production
    default: function() {
        return BuildManifestPlugin;
    },
    getEntrypointFiles: function() {
        return getEntrypointFiles;
    },
    normalizeRewritesForBuildManifest: function() {
        return normalizeRewritesForBuildManifest;
    },
    srcEmptySsgManifest: function() {
        return srcEmptySsgManifest;
    }
});
const _devalue = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/devalue"));
const _webpack = require("next/dist/compiled/webpack/webpack");
const _constants = require("../../../shared/lib/constants");
const _getroutefromentrypoint = /*#__PURE__*/ _interop_require_default(require("../../../server/get-route-from-entrypoint"));
const _nextdropclientpageplugin = require("./next-drop-client-page-plugin");
const _utils = require("../../../shared/lib/router/utils");
const _profilingplugin = require("./profiling-plugin");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const srcEmptySsgManifest = `self.__SSG_MANIFEST=new Set;self.__SSG_MANIFEST_CB&&self.__SSG_MANIFEST_CB()`;
// nodejs: '/static/<build id>/low-priority.js'
function buildNodejsLowPriorityPath(filename, buildId) {
    return `${_constants.CLIENT_STATIC_FILES_PATH}/${buildId}/${filename}`;
}
function createEdgeRuntimeManifest(originAssetMap) {
    const manifestFilenames = [
        "_buildManifest.js",
        "_ssgManifest.js"
    ];
    const assetMap = {
        ...originAssetMap,
        lowPriorityFiles: []
    };
    const manifestDefCode = `self.__BUILD_MANIFEST = ${JSON.stringify(assetMap, null, 2)};\n`;
    // edge lowPriorityFiles item: '"/static/" + process.env.__NEXT_BUILD_ID + "/low-priority.js"'.
    // Since lowPriorityFiles is not fixed and relying on `process.env.__NEXT_BUILD_ID`, we'll produce code creating it dynamically.
    const lowPriorityFilesCode = `self.__BUILD_MANIFEST.lowPriorityFiles = [\n` + manifestFilenames.map((filename)=>{
        return `"/static/" + process.env.__NEXT_BUILD_ID + "/${filename}",\n`;
    }).join(",") + `\n];`;
    return manifestDefCode + lowPriorityFilesCode;
}
function normalizeRewrite(item) {
    return {
        has: item.has,
        source: item.source,
        destination: item.destination
    };
}
function normalizeRewritesForBuildManifest(rewrites) {
    var _rewrites_afterFiles, _rewrites_beforeFiles, _rewrites_fallback;
    return {
        afterFiles: (_rewrites_afterFiles = rewrites.afterFiles) == null ? void 0 : _rewrites_afterFiles.map((item)=>normalizeRewrite(item)),
        beforeFiles: (_rewrites_beforeFiles = rewrites.beforeFiles) == null ? void 0 : _rewrites_beforeFiles.map((item)=>normalizeRewrite(item)),
        fallback: (_rewrites_fallback = rewrites.fallback) == null ? void 0 : _rewrites_fallback.map((item)=>normalizeRewrite(item))
    };
}
// This function takes the asset map generated in BuildManifestPlugin and creates a
// reduced version to send to the client.
function generateClientManifest(compiler, compilation, assetMap, rewrites) {
    const compilationSpan = _profilingplugin.spans.get(compilation) || _profilingplugin.spans.get(compiler);
    const genClientManifestSpan = compilationSpan == null ? void 0 : compilationSpan.traceChild("NextJsBuildManifest-generateClientManifest");
    return genClientManifestSpan == null ? void 0 : genClientManifestSpan.traceFn(()=>{
        const clientManifest = {
            __rewrites: normalizeRewritesForBuildManifest(rewrites)
        };
        const appDependencies = new Set(assetMap.pages["/_app"]);
        const sortedPageKeys = (0, _utils.getSortedRoutes)(Object.keys(assetMap.pages));
        sortedPageKeys.forEach((page)=>{
            const dependencies = assetMap.pages[page];
            if (page === "/_app") return;
            // Filter out dependencies in the _app entry, because those will have already
            // been loaded by the client prior to a navigation event
            const filteredDeps = dependencies.filter((dep)=>!appDependencies.has(dep));
            // The manifest can omit the page if it has no requirements
            if (filteredDeps.length) {
                clientManifest[page] = filteredDeps;
            }
        });
        // provide the sorted pages as an array so we don't rely on the object's keys
        // being in order and we don't slow down look-up time for page assets
        clientManifest.sortedPages = sortedPageKeys;
        return (0, _devalue.default)(clientManifest);
    });
}
function getEntrypointFiles(entrypoint) {
    return (entrypoint == null ? void 0 : entrypoint.getFiles().filter((file)=>{
        // We don't want to include `.hot-update.js` files into the initial page
        return /(?<!\.hot-update)\.(js|css)($|\?)/.test(file);
    }).map((file)=>file.replace(/\\/g, "/"))) ?? [];
}
const processRoute = (r)=>{
    const rewrite = {
        ...r
    };
    // omit external rewrite destinations since these aren't
    // handled client-side
    if (!rewrite.destination.startsWith("/")) {
        delete rewrite.destination;
    }
    return rewrite;
};
class BuildManifestPlugin {
    constructor(options){
        this.buildId = options.buildId;
        this.isDevFallback = !!options.isDevFallback;
        this.rewrites = {
            beforeFiles: [],
            afterFiles: [],
            fallback: []
        };
        this.appDirEnabled = options.appDirEnabled;
        this.rewrites.beforeFiles = options.rewrites.beforeFiles.map(processRoute);
        this.rewrites.afterFiles = options.rewrites.afterFiles.map(processRoute);
        this.rewrites.fallback = options.rewrites.fallback.map(processRoute);
        this.exportRuntime = !!options.exportRuntime;
    }
    createAssets(compiler, compilation, assets) {
        const compilationSpan = _profilingplugin.spans.get(compilation) || _profilingplugin.spans.get(compiler);
        const createAssetsSpan = compilationSpan == null ? void 0 : compilationSpan.traceChild("NextJsBuildManifest-createassets");
        return createAssetsSpan == null ? void 0 : createAssetsSpan.traceFn(()=>{
            const entrypoints = compilation.entrypoints;
            const assetMap = {
                polyfillFiles: [],
                devFiles: [],
                ampDevFiles: [],
                lowPriorityFiles: [],
                rootMainFiles: [],
                pages: {
                    "/_app": []
                },
                ampFirstPages: []
            };
            const ampFirstEntryNames = _nextdropclientpageplugin.ampFirstEntryNamesMap.get(compilation);
            if (ampFirstEntryNames) {
                for (const entryName of ampFirstEntryNames){
                    const pagePath = (0, _getroutefromentrypoint.default)(entryName);
                    if (!pagePath) {
                        continue;
                    }
                    assetMap.ampFirstPages.push(pagePath);
                }
            }
            const mainFiles = new Set(getEntrypointFiles(entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN)));
            if (this.appDirEnabled) {
                assetMap.rootMainFiles = [
                    ...new Set(getEntrypointFiles(entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP)))
                ];
            }
            const compilationAssets = compilation.getAssets();
            assetMap.polyfillFiles = compilationAssets.filter((p)=>{
                // Ensure only .js files are passed through
                if (!p.name.endsWith(".js")) {
                    return false;
                }
                return p.info && _constants.CLIENT_STATIC_FILES_RUNTIME_POLYFILLS_SYMBOL in p.info;
            }).map((v)=>v.name);
            assetMap.devFiles = getEntrypointFiles(entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH)).filter((file)=>!mainFiles.has(file));
            assetMap.ampDevFiles = getEntrypointFiles(entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_AMP));
            for (const entrypoint of compilation.entrypoints.values()){
                if (_constants.SYSTEM_ENTRYPOINTS.has(entrypoint.name)) continue;
                const pagePath = (0, _getroutefromentrypoint.default)(entrypoint.name);
                if (!pagePath) {
                    continue;
                }
                const filesForPage = getEntrypointFiles(entrypoint);
                assetMap.pages[pagePath] = [
                    ...new Set([
                        ...mainFiles,
                        ...filesForPage
                    ])
                ];
            }
            if (!this.isDevFallback) {
                // Add the runtime build manifest file (generated later in this file)
                // as a dependency for the app. If the flag is false, the file won't be
                // downloaded by the client.
                const buildManifestPath = buildNodejsLowPriorityPath("_buildManifest.js", this.buildId);
                const ssgManifestPath = buildNodejsLowPriorityPath("_ssgManifest.js", this.buildId);
                assetMap.lowPriorityFiles.push(buildManifestPath, ssgManifestPath);
                assets[ssgManifestPath] = new _webpack.sources.RawSource(srcEmptySsgManifest);
            }
            assetMap.pages = Object.keys(assetMap.pages).sort().reduce(// eslint-disable-next-line
            (a, c)=>(a[c] = assetMap.pages[c], a), {});
            let buildManifestName = _constants.BUILD_MANIFEST;
            if (this.isDevFallback) {
                buildManifestName = `fallback-${_constants.BUILD_MANIFEST}`;
            }
            assets[buildManifestName] = new _webpack.sources.RawSource(JSON.stringify(assetMap, null, 2));
            assets[`server/${_constants.MIDDLEWARE_BUILD_MANIFEST}.js`] = new _webpack.sources.RawSource(`${createEdgeRuntimeManifest(assetMap)}`);
            if (!this.isDevFallback) {
                const clientManifestPath = `${_constants.CLIENT_STATIC_FILES_PATH}/${this.buildId}/_buildManifest.js`;
                assets[clientManifestPath] = new _webpack.sources.RawSource(`self.__BUILD_MANIFEST = ${generateClientManifest(compiler, compilation, assetMap, this.rewrites)};self.__BUILD_MANIFEST_CB && self.__BUILD_MANIFEST_CB()`);
            }
            return assets;
        });
    }
    apply(compiler) {
        compiler.hooks.make.tap("NextJsBuildManifest", (compilation)=>{
            compilation.hooks.processAssets.tap({
                name: "NextJsBuildManifest",
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>{
                this.createAssets(compiler, compilation, assets);
            });
        });
        return;
    }
}

//# sourceMappingURL=build-manifest-plugin.js.map