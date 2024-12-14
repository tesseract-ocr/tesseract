"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return ReactDevOverlay;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _bus = /*#__PURE__*/ _interop_require_wildcard._(require("./bus"));
const _ShadowPortal = require("../internal/components/ShadowPortal");
const _BuildError = require("../internal/container/BuildError");
const _Errors = require("../internal/container/Errors");
const _ErrorBoundary = require("./ErrorBoundary");
const _Base = require("../internal/styles/Base");
const _ComponentStyles = require("../internal/styles/ComponentStyles");
const _CssReset = require("../internal/styles/CssReset");
const _shared = require("../shared");
const shouldPreventDisplay = (errorType, preventType)=>{
    if (!preventType || !errorType) {
        return false;
    }
    return preventType.includes(errorType);
};
function ReactDevOverlay(param) {
    let { children, preventDisplay, globalOverlay } = param;
    const [state, dispatch] = (0, _shared.useErrorOverlayReducer)();
    _react.useEffect(()=>{
        _bus.on(dispatch);
        return function() {
            _bus.off(dispatch);
        };
    }, [
        dispatch
    ]);
    const onComponentError = _react.useCallback((_error, _componentStack)=>{
    // TODO: special handling
    }, []);
    const hasBuildError = state.buildError != null;
    const hasRuntimeErrors = Boolean(state.errors.length);
    const errorType = hasBuildError ? 'build' : hasRuntimeErrors ? 'runtime' : null;
    const isMounted = errorType !== null;
    const displayPrevented = shouldPreventDisplay(errorType, preventDisplay);
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)(_ErrorBoundary.ErrorBoundary, {
                globalOverlay: globalOverlay,
                isMounted: isMounted,
                onError: onComponentError,
                children: children != null ? children : null
            }),
            isMounted ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_ShadowPortal.ShadowPortal, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_CssReset.CssReset, {}),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_Base.Base, {}),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_ComponentStyles.ComponentStyles, {}),
                    displayPrevented ? null : hasBuildError ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_BuildError.BuildError, {
                        message: state.buildError,
                        versionInfo: state.versionInfo
                    }) : hasRuntimeErrors ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_Errors.Errors, {
                        isAppDir: false,
                        errors: state.errors,
                        versionInfo: state.versionInfo,
                        initialDisplayState: 'fullscreen'
                    }) : undefined
                ]
            }) : undefined
        ]
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=ReactDevOverlay.js.map