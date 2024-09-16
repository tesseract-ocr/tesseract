import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import * as React from "react";
import * as Bus from "./bus";
import { ShadowPortal } from "../internal/components/ShadowPortal";
import { BuildError } from "../internal/container/BuildError";
import { Errors } from "../internal/container/Errors";
import { ErrorBoundary } from "./ErrorBoundary";
import { Base } from "../internal/styles/Base";
import { ComponentStyles } from "../internal/styles/ComponentStyles";
import { CssReset } from "../internal/styles/CssReset";
import { useErrorOverlayReducer } from "../shared";
const shouldPreventDisplay = (errorType, preventType)=>{
    if (!preventType || !errorType) {
        return false;
    }
    return preventType.includes(errorType);
};
export default function ReactDevOverlay(param) {
    let { children, preventDisplay, globalOverlay } = param;
    const [state, dispatch] = useErrorOverlayReducer();
    React.useEffect(()=>{
        Bus.on(dispatch);
        return function() {
            Bus.off(dispatch);
        };
    }, [
        dispatch
    ]);
    const onComponentError = React.useCallback((_error, _componentStack)=>{
    // TODO: special handling
    }, []);
    const hasBuildError = state.buildError != null;
    const hasRuntimeErrors = Boolean(state.errors.length);
    const errorType = hasBuildError ? "build" : hasRuntimeErrors ? "runtime" : null;
    const isMounted = errorType !== null;
    const displayPrevented = shouldPreventDisplay(errorType, preventDisplay);
    return /*#__PURE__*/ _jsxs(_Fragment, {
        children: [
            /*#__PURE__*/ _jsx(ErrorBoundary, {
                globalOverlay: globalOverlay,
                isMounted: isMounted,
                onError: onComponentError,
                children: children != null ? children : null
            }),
            isMounted ? /*#__PURE__*/ _jsxs(ShadowPortal, {
                children: [
                    /*#__PURE__*/ _jsx(CssReset, {}),
                    /*#__PURE__*/ _jsx(Base, {}),
                    /*#__PURE__*/ _jsx(ComponentStyles, {}),
                    displayPrevented ? null : hasBuildError ? /*#__PURE__*/ _jsx(BuildError, {
                        message: state.buildError,
                        versionInfo: state.versionInfo
                    }) : hasRuntimeErrors ? /*#__PURE__*/ _jsx(Errors, {
                        isAppDir: false,
                        errors: state.errors,
                        versionInfo: state.versionInfo,
                        initialDisplayState: "fullscreen"
                    }) : undefined
                ]
            }) : undefined
        ]
    });
}

//# sourceMappingURL=ReactDevOverlay.js.map