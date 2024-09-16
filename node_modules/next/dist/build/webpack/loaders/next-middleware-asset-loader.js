"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    raw: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return MiddlewareAssetLoader;
    },
    raw: function() {
        return raw;
    }
});
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _getmodulebuildinfo = require("./get-module-build-info");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function MiddlewareAssetLoader(source) {
    const name = _loaderutils3.default.interpolateName(this, "[name].[hash].[ext]", {
        context: this.rootContext,
        content: source
    });
    const filePath = `edge-chunks/asset_${name}`;
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.nextAssetMiddlewareBinding = {
        filePath: `server/${filePath}`,
        name
    };
    this.emitFile(filePath, source);
    return `module.exports = ${JSON.stringify(`blob:${name}`)}`;
}
const raw = true;

//# sourceMappingURL=next-middleware-asset-loader.js.map