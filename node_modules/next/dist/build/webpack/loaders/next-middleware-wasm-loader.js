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
        return MiddlewareWasmLoader;
    },
    raw: function() {
        return raw;
    }
});
const _getmodulebuildinfo = require("./get-module-build-info");
const _crypto = /*#__PURE__*/ _interop_require_default(require("crypto"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function sha1(source) {
    return _crypto.default.createHash("sha1").update(source).digest("hex");
}
function MiddlewareWasmLoader(source) {
    const name = `wasm_${sha1(source)}`;
    const filePath = `edge-chunks/${name}.wasm`;
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.nextWasmMiddlewareBinding = {
        filePath: `server/${filePath}`,
        name
    };
    this.emitFile(`/${filePath}`, source, null);
    return `module.exports = ${name};`;
}
const raw = true;

//# sourceMappingURL=next-middleware-wasm-loader.js.map