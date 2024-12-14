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
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default._(require("react"));
const _ShadowPortal = require("../internal/components/ShadowPortal");
const _BuildError = require("../internal/container/BuildError");
const _Errors = require("../internal/container/Errors");
const _StaticIndicator = require("../internal/container/StaticIndicator");
const _Base = require("../internal/styles/Base");
const _ComponentStyles = require("../internal/styles/ComponentStyles");
const _CssReset = require("../internal/styles/CssReset");
const _rootlayoutmissingtagserror = require("../internal/container/root-layout-missing-tags-error");
const _runtimeerrorhandler = require("../internal/helpers/runtime-error-handler");
class ReactDevOverlay extends _react.default.PureComponent {
    static getDerivedStateFromError(error) {
        if (!error.stack) return {
            isReactError: false
        };
        _runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError = true;
        return {
            isReactError: true
        };
    }
    render() {
        var _state_rootLayoutMissingTags;
        const { state, children, dispatcher } = this.props;
        const { isReactError } = this.state;
        const hasBuildError = state.buildError != null;
        const hasRuntimeErrors = Boolean(state.errors.length);
        const hasStaticIndicator = state.staticIndicator;
        const debugInfo = state.debugInfo;
        return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
            children: [
                isReactError ? /*#__PURE__*/ (0, _jsxruntime.jsxs)("html", {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("head", {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("body", {})
                    ]
                }) : children,
                /*#__PURE__*/ (0, _jsxruntime.jsxs)(_ShadowPortal.ShadowPortal, {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_CssReset.CssReset, {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_Base.Base, {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_ComponentStyles.ComponentStyles, {}),
                        ((_state_rootLayoutMissingTags = state.rootLayoutMissingTags) == null ? void 0 : _state_rootLayoutMissingTags.length) ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_rootlayoutmissingtagserror.RootLayoutMissingTagsError, {
                            missingTags: state.rootLayoutMissingTags
                        }) : hasBuildError ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_BuildError.BuildError, {
                            message: state.buildError,
                            versionInfo: state.versionInfo
                        }) : /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                            children: [
                                hasRuntimeErrors ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_Errors.Errors, {
                                    isAppDir: true,
                                    initialDisplayState: isReactError ? 'fullscreen' : 'minimized',
                                    errors: state.errors,
                                    versionInfo: state.versionInfo,
                                    hasStaticIndicator: hasStaticIndicator,
                                    debugInfo: debugInfo
                                }) : null,
                                hasStaticIndicator && /*#__PURE__*/ (0, _jsxruntime.jsx)(_StaticIndicator.StaticIndicator, {
                                    dispatcher: dispatcher
                                })
                            ]
                        })
                    ]
                })
            ]
        });
    }
    constructor(...args){
        super(...args), this.state = {
            isReactError: false
        };
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=ReactDevOverlay.js.map