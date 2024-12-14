"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "NextJsRequireCacheHotReloader", {
    enumerable: true,
    get: function() {
        return NextJsRequireCacheHotReloader;
    }
});
const _requirecache = require("../../../server/dev/require-cache");
const _sandbox = require("../../../server/web/sandbox");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const RUNTIME_NAMES = [
    'webpack-runtime',
    'webpack-api-runtime'
];
const PLUGIN_NAME = 'NextJsRequireCacheHotReloader';
class NextJsRequireCacheHotReloader {
    constructor(opts){
        this.prevAssets = null;
        this.serverComponents = opts.serverComponents;
    }
    apply(compiler) {
        compiler.hooks.assetEmitted.tap(PLUGIN_NAME, (_file, { targetPath })=>{
            // Clear module context in this process
            (0, _sandbox.clearModuleContext)(targetPath);
            (0, _requirecache.deleteCache)(targetPath);
        });
        compiler.hooks.afterEmit.tapPromise(PLUGIN_NAME, async (compilation)=>{
            for (const name of RUNTIME_NAMES){
                const runtimeChunkPath = _path.default.join(compilation.outputOptions.path, `${name}.js`);
                (0, _requirecache.deleteCache)(runtimeChunkPath);
            }
            // we need to make sure to clear all server entries from cache
            // since they can have a stale webpack-runtime cache
            // which needs to always be in-sync
            const entries = [
                ...compilation.entries.keys()
            ].filter((entry)=>{
                const isAppPath = entry.toString().startsWith('app/');
                return entry.toString().startsWith('pages/') || isAppPath;
            });
            for (const page of entries){
                const outputPath = _path.default.join(compilation.outputOptions.path, page + '.js');
                (0, _requirecache.deleteCache)(outputPath);
            }
        });
    }
}

//# sourceMappingURL=nextjs-require-cache-hot-reloader.js.map