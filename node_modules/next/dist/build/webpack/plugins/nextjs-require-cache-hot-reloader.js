"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    NextJsRequireCacheHotReloader: null,
    deleteAppClientCache: null,
    deleteCache: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    NextJsRequireCacheHotReloader: function() {
        return NextJsRequireCacheHotReloader;
    },
    deleteAppClientCache: function() {
        return deleteAppClientCache;
    },
    deleteCache: function() {
        return deleteCache;
    }
});
const _sandbox = require("../../../server/web/sandbox");
const _realpath = require("../../../lib/realpath");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../../lib/is-error"));
const _loadmanifest = require("../../../server/load-manifest");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const originModules = [
    require.resolve("../../../server/require"),
    require.resolve("../../../server/load-components"),
    require.resolve("../../../server/next-server"),
    require.resolve("next/dist/compiled/next-server/app-page.runtime.dev.js"),
    require.resolve("next/dist/compiled/next-server/app-route.runtime.dev.js"),
    require.resolve("next/dist/compiled/next-server/pages.runtime.dev.js"),
    require.resolve("next/dist/compiled/next-server/pages-api.runtime.dev.js")
];
const RUNTIME_NAMES = [
    "webpack-runtime",
    "webpack-api-runtime"
];
function deleteFromRequireCache(filePath) {
    try {
        filePath = (0, _realpath.realpathSync)(filePath);
    } catch (e) {
        if ((0, _iserror.default)(e) && e.code !== "ENOENT") throw e;
    }
    const mod = require.cache[filePath];
    if (mod) {
        // remove the child reference from the originModules
        for (const originModule of originModules){
            const parent = require.cache[originModule];
            if (parent) {
                const idx = parent.children.indexOf(mod);
                if (idx >= 0) parent.children.splice(idx, 1);
            }
        }
        // remove parent references from external modules
        for (const child of mod.children){
            child.parent = null;
        }
        delete require.cache[filePath];
        return true;
    }
    return false;
}
function deleteAppClientCache() {
    deleteFromRequireCache(require.resolve("next/dist/compiled/next-server/app-page.runtime.dev.js"));
    deleteFromRequireCache(require.resolve("next/dist/compiled/next-server/app-page-experimental.runtime.dev.js"));
}
function deleteCache(filePath) {
    // try to clear it from the fs cache
    (0, _loadmanifest.clearManifestCache)(filePath);
    deleteFromRequireCache(filePath);
}
const PLUGIN_NAME = "NextJsRequireCacheHotReloader";
class NextJsRequireCacheHotReloader {
    constructor(opts){
        this.prevAssets = null;
        this.serverComponents = opts.serverComponents;
    }
    apply(compiler) {
        compiler.hooks.assetEmitted.tap(PLUGIN_NAME, (_file, { targetPath })=>{
            // Clear module context in this process
            (0, _sandbox.clearModuleContext)(targetPath);
            deleteCache(targetPath);
        });
        compiler.hooks.afterEmit.tapPromise(PLUGIN_NAME, async (compilation)=>{
            for (const name of RUNTIME_NAMES){
                const runtimeChunkPath = _path.default.join(compilation.outputOptions.path, `${name}.js`);
                deleteCache(runtimeChunkPath);
            }
            // we need to make sure to clear all server entries from cache
            // since they can have a stale webpack-runtime cache
            // which needs to always be in-sync
            let hasAppEntry = false;
            const entries = [
                ...compilation.entries.keys()
            ].filter((entry)=>{
                const isAppPath = entry.toString().startsWith("app/");
                if (isAppPath) hasAppEntry = true;
                return entry.toString().startsWith("pages/") || isAppPath;
            });
            if (hasAppEntry) {
                deleteAppClientCache();
            }
            for (const page of entries){
                const outputPath = _path.default.join(compilation.outputOptions.path, page + ".js");
                deleteCache(outputPath);
            }
        });
    }
}

//# sourceMappingURL=nextjs-require-cache-hot-reloader.js.map