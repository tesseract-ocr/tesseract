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
const _shared = require("../shared");
const _ShadowPortal = require("../internal/components/ShadowPortal");
const _BuildError = require("../internal/container/BuildError");
const _Errors = require("../internal/container/Errors");
const _parseStack = require("../internal/helpers/parseStack");
const _Base = require("../internal/styles/Base");
const _ComponentStyles = require("../internal/styles/ComponentStyles");
const _CssReset = require("../internal/styles/CssReset");
const _rootlayoutmissingtagserror = require("../internal/container/root-layout-missing-tags-error");
class ReactDevOverlay extends _react.PureComponent {
    static getDerivedStateFromError(error) {
        if (!error.stack) return {
            reactError: null
        };
        return {
            reactError: {
                id: 0,
                event: {
                    type: _shared.ACTION_UNHANDLED_ERROR,
                    reason: error,
                    frames: (0, _parseStack.parseStack)(error.stack)
                }
            }
        };
    }
    componentDidCatch(componentErr) {
        this.props.onReactError(componentErr);
    }
    render() {
        var _state_rootLayoutMissingTags, _state_rootLayoutMissingTags1;
        const { state, children } = this.props;
        const { reactError } = this.state;
        const hasBuildError = state.buildError != null;
        const hasRuntimeErrors = Boolean(state.errors.length);
        const hasMissingTags = Boolean((_state_rootLayoutMissingTags = state.rootLayoutMissingTags) == null ? void 0 : _state_rootLayoutMissingTags.length);
        const isMounted = hasBuildError || hasRuntimeErrors || reactError || hasMissingTags;
        return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
            children: [
                reactError ? /*#__PURE__*/ (0, _jsxruntime.jsxs)("html", {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("head", {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("body", {})
                    ]
                }) : children,
                isMounted ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_ShadowPortal.ShadowPortal, {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_CssReset.CssReset, {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_Base.Base, {}),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)(_ComponentStyles.ComponentStyles, {}),
                        ((_state_rootLayoutMissingTags1 = state.rootLayoutMissingTags) == null ? void 0 : _state_rootLayoutMissingTags1.length) ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_rootlayoutmissingtagserror.RootLayoutMissingTagsError, {
                            missingTags: state.rootLayoutMissingTags
                        }) : hasBuildError ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_BuildError.BuildError, {
                            message: state.buildError,
                            versionInfo: state.versionInfo
                        }) : reactError ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_Errors.Errors, {
                            isAppDir: true,
                            versionInfo: state.versionInfo,
                            initialDisplayState: "fullscreen",
                            errors: [
                                reactError
                            ]
                        }) : hasRuntimeErrors ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_Errors.Errors, {
                            isAppDir: true,
                            initialDisplayState: "minimized",
                            errors: state.errors,
                            versionInfo: state.versionInfo
                        }) : undefined
                    ]
                }) : undefined
            ]
        });
    }
    constructor(...args){
        super(...args);
        this.state = {
            reactError: null
        };
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=ReactDevOverlay.js.map