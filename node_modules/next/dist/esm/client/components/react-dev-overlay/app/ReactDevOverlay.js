import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import * as React from "react";
import { ACTION_UNHANDLED_ERROR } from "../shared";
import { ShadowPortal } from "../internal/components/ShadowPortal";
import { BuildError } from "../internal/container/BuildError";
import { Errors } from "../internal/container/Errors";
import { parseStack } from "../internal/helpers/parseStack";
import { Base } from "../internal/styles/Base";
import { ComponentStyles } from "../internal/styles/ComponentStyles";
import { CssReset } from "../internal/styles/CssReset";
import { RootLayoutMissingTagsError } from "../internal/container/root-layout-missing-tags-error";
class ReactDevOverlay extends React.PureComponent {
    static getDerivedStateFromError(error) {
        if (!error.stack) return {
            reactError: null
        };
        return {
            reactError: {
                id: 0,
                event: {
                    type: ACTION_UNHANDLED_ERROR,
                    reason: error,
                    frames: parseStack(error.stack)
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
        return /*#__PURE__*/ _jsxs(_Fragment, {
            children: [
                reactError ? /*#__PURE__*/ _jsxs("html", {
                    children: [
                        /*#__PURE__*/ _jsx("head", {}),
                        /*#__PURE__*/ _jsx("body", {})
                    ]
                }) : children,
                isMounted ? /*#__PURE__*/ _jsxs(ShadowPortal, {
                    children: [
                        /*#__PURE__*/ _jsx(CssReset, {}),
                        /*#__PURE__*/ _jsx(Base, {}),
                        /*#__PURE__*/ _jsx(ComponentStyles, {}),
                        ((_state_rootLayoutMissingTags1 = state.rootLayoutMissingTags) == null ? void 0 : _state_rootLayoutMissingTags1.length) ? /*#__PURE__*/ _jsx(RootLayoutMissingTagsError, {
                            missingTags: state.rootLayoutMissingTags
                        }) : hasBuildError ? /*#__PURE__*/ _jsx(BuildError, {
                            message: state.buildError,
                            versionInfo: state.versionInfo
                        }) : reactError ? /*#__PURE__*/ _jsx(Errors, {
                            isAppDir: true,
                            versionInfo: state.versionInfo,
                            initialDisplayState: "fullscreen",
                            errors: [
                                reactError
                            ]
                        }) : hasRuntimeErrors ? /*#__PURE__*/ _jsx(Errors, {
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
export { ReactDevOverlay as default };

//# sourceMappingURL=ReactDevOverlay.js.map