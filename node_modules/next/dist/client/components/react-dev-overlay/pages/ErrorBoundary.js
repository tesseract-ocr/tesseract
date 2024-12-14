"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ErrorBoundary", {
    enumerable: true,
    get: function() {
        return ErrorBoundary;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
class ErrorBoundary extends _react.PureComponent {
    static getDerivedStateFromError(error) {
        return {
            error
        };
    }
    componentDidCatch(error, // Loosely typed because it depends on the React version and was
    // accidentally excluded in some versions.
    errorInfo) {
        this.props.onError(error, (errorInfo == null ? void 0 : errorInfo.componentStack) || null);
        if (!this.props.globalOverlay) {
            this.setState({
                error
            });
        }
    }
    // Explicit type is needed to avoid the generated `.d.ts` having a wide return type that could be specific to the `@types/react` version.
    render() {
        // The component has to be unmounted or else it would continue to error
        return this.state.error || this.props.globalOverlay && this.props.isMounted ? // When the overlay is global for the application and it wraps a component rendering `<html>`
        // we have to render the html shell otherwise the shadow root will not be able to attach
        this.props.globalOverlay ? /*#__PURE__*/ (0, _jsxruntime.jsxs)("html", {
            children: [
                /*#__PURE__*/ (0, _jsxruntime.jsx)("head", {}),
                /*#__PURE__*/ (0, _jsxruntime.jsx)("body", {})
            ]
        }) : null : this.props.children;
    }
    constructor(...args){
        super(...args), this.state = {
            error: null
        };
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=ErrorBoundary.js.map