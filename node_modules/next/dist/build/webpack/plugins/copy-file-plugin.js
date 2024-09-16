"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "CopyFilePlugin", {
    enumerable: true,
    get: function() {
        return CopyFilePlugin;
    }
});
const _fs = require("fs");
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _webpack = require("next/dist/compiled/webpack/webpack");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const PLUGIN_NAME = "CopyFilePlugin";
class CopyFilePlugin {
    constructor({ filePath, cacheKey, name, info }){
        this.filePath = filePath;
        this.cacheKey = cacheKey;
        this.name = name;
        this.info = info;
    }
    apply(compiler) {
        compiler.hooks.thisCompilation.tap(PLUGIN_NAME, (compilation)=>{
            const cache = compilation.getCache("CopyFilePlugin");
            const hook = compilation.hooks.processAssets;
            hook.tapPromise({
                name: PLUGIN_NAME,
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, async ()=>{
                if (cache) {
                    const cachedResult = await cache.getPromise(this.filePath, this.cacheKey);
                    if (cachedResult) {
                        const { file, source } = cachedResult;
                        compilation.emitAsset(file, source, {
                            ...this.info
                        });
                        return;
                    }
                }
                const content = await _fs.promises.readFile(this.filePath, "utf8");
                const file = _loaderutils3.default.interpolateName({
                    resourcePath: this.filePath
                }, this.name, {
                    content,
                    context: compiler.context
                });
                const source = new _webpack.sources.RawSource(content);
                if (cache) {
                    await cache.storePromise(this.filePath, this.cacheKey, {
                        file,
                        source
                    });
                }
                compilation.emitAsset(file, source, {
                    ...this.info
                });
            });
        });
    }
}

//# sourceMappingURL=copy-file-plugin.js.map