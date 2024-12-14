import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import React from 'react';
import { ShadowPortal } from '../internal/components/ShadowPortal';
import { BuildError } from '../internal/container/BuildError';
import { Errors } from '../internal/container/Errors';
import { StaticIndicator } from '../internal/container/StaticIndicator';
import { Base } from '../internal/styles/Base';
import { ComponentStyles } from '../internal/styles/ComponentStyles';
import { CssReset } from '../internal/styles/CssReset';
import { RootLayoutMissingTagsError } from '../internal/container/root-layout-missing-tags-error';
import { RuntimeErrorHandler } from '../internal/helpers/runtime-error-handler';
class ReactDevOverlay extends React.PureComponent {
    static getDerivedStateFromError(error) {
        if (!error.stack) return {
            isReactError: false
        };
        RuntimeErrorHandler.hadRuntimeError = true;
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
        return /*#__PURE__*/ _jsxs(_Fragment, {
            children: [
                isReactError ? /*#__PURE__*/ _jsxs("html", {
                    children: [
                        /*#__PURE__*/ _jsx("head", {}),
                        /*#__PURE__*/ _jsx("body", {})
                    ]
                }) : children,
                /*#__PURE__*/ _jsxs(ShadowPortal, {
                    children: [
                        /*#__PURE__*/ _jsx(CssReset, {}),
                        /*#__PURE__*/ _jsx(Base, {}),
                        /*#__PURE__*/ _jsx(ComponentStyles, {}),
                        ((_state_rootLayoutMissingTags = state.rootLayoutMissingTags) == null ? void 0 : _state_rootLayoutMissingTags.length) ? /*#__PURE__*/ _jsx(RootLayoutMissingTagsError, {
                            missingTags: state.rootLayoutMissingTags
                        }) : hasBuildError ? /*#__PURE__*/ _jsx(BuildError, {
                            message: state.buildError,
                            versionInfo: state.versionInfo
                        }) : /*#__PURE__*/ _jsxs(_Fragment, {
                            children: [
                                hasRuntimeErrors ? /*#__PURE__*/ _jsx(Errors, {
                                    isAppDir: true,
                                    initialDisplayState: isReactError ? 'fullscreen' : 'minimized',
                                    errors: state.errors,
                                    versionInfo: state.versionInfo,
                                    hasStaticIndicator: hasStaticIndicator,
                                    debugInfo: debugInfo
                                }) : null,
                                hasStaticIndicator && /*#__PURE__*/ _jsx(StaticIndicator, {
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
export { ReactDevOverlay as default };

//# sourceMappingURL=ReactDevOverlay.js.map