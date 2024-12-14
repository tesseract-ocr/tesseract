"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "useOnClickOutside", {
    enumerable: true,
    get: function() {
        return useOnClickOutside;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
function useOnClickOutside(el, handler) {
    _react.useEffect(()=>{
        if (el == null || handler == null) {
            return;
        }
        const listener = (e)=>{
            // Do nothing if clicking ref's element or descendent elements
            if (!el || el.contains(e.target)) {
                return;
            }
            handler(e);
        };
        const root = el.getRootNode();
        root.addEventListener('mousedown', listener);
        root.addEventListener('touchstart', listener, {
            passive: false
        });
        return function() {
            root.removeEventListener('mousedown', listener);
            root.removeEventListener('touchstart', listener);
        };
    }, [
        handler,
        el
    ]);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=use-on-click-outside.js.map