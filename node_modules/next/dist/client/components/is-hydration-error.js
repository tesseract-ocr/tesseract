"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isHydrationError", {
    enumerable: true,
    get: function() {
        return isHydrationError;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _iserror = /*#__PURE__*/ _interop_require_default._(require("../../lib/is-error"));
const hydrationErrorRegex = /hydration failed|while hydrating|content does not match|did not match/i;
function isHydrationError(error) {
    return (0, _iserror.default)(error) && hydrationErrorRegex.test(error.message);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=is-hydration-error.js.map