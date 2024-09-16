"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "useAmp", {
    enumerable: true,
    get: function() {
        return useAmp;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _react = /*#__PURE__*/ _interop_require_default._(require("react"));
const _ampcontextsharedruntime = require("./amp-context.shared-runtime");
const _ampmode = require("./amp-mode");
function useAmp() {
    // Don't assign the context value to a variable to save bytes
    return (0, _ampmode.isInAmpMode)(_react.default.useContext(_ampcontextsharedruntime.AmpStateContext));
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=amp.js.map