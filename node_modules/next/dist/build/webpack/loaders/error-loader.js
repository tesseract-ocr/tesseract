"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _picocolors = require("../../../lib/picocolors");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const ErrorLoader = function() {
    var _this__module_issuer, _this__module, _this__compiler;
    // @ts-ignore exists
    const options = this.getOptions() || {};
    const { reason = 'An unknown error has occurred' } = options;
    // @ts-expect-error
    const resource = ((_this__module = this._module) == null ? void 0 : (_this__module_issuer = _this__module.issuer) == null ? void 0 : _this__module_issuer.resource) ?? null;
    const context = this.rootContext ?? ((_this__compiler = this._compiler) == null ? void 0 : _this__compiler.context);
    const issuer = resource ? context ? _path.default.relative(context, resource) : resource : null;
    const err = new Error(reason + (issuer ? `\nLocation: ${(0, _picocolors.cyan)(issuer)}` : ''));
    this.emitError(err);
};
const _default = ErrorLoader;

//# sourceMappingURL=error-loader.js.map