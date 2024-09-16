import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import * as React from "react";
export class ErrorBoundary extends React.PureComponent {
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
    // Explicit type is needed to avoid the generated `.d.ts` having a wide return type that could be specific the the `@types/react` version.
    render() {
        // The component has to be unmounted or else it would continue to error
        return this.state.error || this.props.globalOverlay && this.props.isMounted ? // When the overlay is global for the application and it wraps a component rendering `<html>`
        // we have to render the html shell otherwise the shadow root will not be able to attach
        this.props.globalOverlay ? /*#__PURE__*/ _jsxs("html", {
            children: [
                /*#__PURE__*/ _jsx("head", {}),
                /*#__PURE__*/ _jsx("body", {})
            ]
        }) : null : this.props.children;
    }
    constructor(...args){
        super(...args);
        this.state = {
            error: null
        };
    }
}

//# sourceMappingURL=ErrorBoundary.js.map