"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "Dialog", {
    enumerable: true,
    get: function() {
        return Dialog;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _useonclickoutside = require("../../hooks/use-on-click-outside");
const Dialog = function Dialog(param) {
    let { children, type, onClose, ...props } = param;
    const [dialog, setDialog] = _react.useState(null);
    const [role, setRole] = _react.useState(typeof document !== 'undefined' && document.hasFocus() ? 'dialog' : undefined);
    const onDialog = _react.useCallback((node)=>{
        setDialog(node);
    }, []);
    (0, _useonclickoutside.useOnClickOutside)(dialog, (e)=>{
        e.preventDefault();
        return onClose == null ? void 0 : onClose();
    });
    // Make HTMLElements with `role=link` accessible to be triggered by the
    // keyboard, i.e. [Enter].
    _react.useEffect(()=>{
        if (dialog == null) {
            return;
        }
        const root = dialog.getRootNode();
        // Always true, but we do this for TypeScript:
        if (!(root instanceof ShadowRoot)) {
            return;
        }
        const shadowRoot = root;
        function handler(e) {
            const el = shadowRoot.activeElement;
            if (e.key === 'Enter' && el instanceof HTMLElement && el.getAttribute('role') === 'link') {
                e.preventDefault();
                e.stopPropagation();
                el.click();
            }
        }
        function handleFocus() {
            // safari will force itself as the active application when a background page triggers any sort of autofocus
            // this is a workaround to only set the dialog role if the document has focus
            setRole(document.hasFocus() ? 'dialog' : undefined);
        }
        shadowRoot.addEventListener('keydown', handler);
        window.addEventListener('focus', handleFocus);
        window.addEventListener('blur', handleFocus);
        return ()=>{
            shadowRoot.removeEventListener('keydown', handler);
            window.removeEventListener('focus', handleFocus);
            window.removeEventListener('blur', handleFocus);
        };
    }, [
        dialog
    ]);
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
        ref: onDialog,
        "data-nextjs-dialog": true,
        tabIndex: -1,
        role: role,
        "aria-labelledby": props['aria-labelledby'],
        "aria-describedby": props['aria-describedby'],
        "aria-modal": "true",
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)("div", {
                "data-nextjs-dialog-banner": true,
                className: "banner-" + type
            }),
            children
        ]
    });
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=Dialog.js.map