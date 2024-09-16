import { cyan } from "../../../lib/picocolors";
import path from "path";
const ErrorLoader = function() {
    var _this__module_issuer, _this__module, _this__compiler;
    // @ts-ignore exists
    const options = this.getOptions() || {};
    const { reason = "An unknown error has occurred" } = options;
    // @ts-expect-error
    const resource = ((_this__module = this._module) == null ? void 0 : (_this__module_issuer = _this__module.issuer) == null ? void 0 : _this__module_issuer.resource) ?? null;
    const context = this.rootContext ?? ((_this__compiler = this._compiler) == null ? void 0 : _this__compiler.context);
    const issuer = resource ? context ? path.relative(context, resource) : resource : null;
    const err = new Error(reason + (issuer ? `\nLocation: ${cyan(issuer)}` : ""));
    this.emitError(err);
};
export default ErrorLoader;

//# sourceMappingURL=error-loader.js.map