import React, { type JSX } from 'react';
export type ErrorComponent = React.ComponentType<{
    error: Error;
    reset: () => void;
}>;
export interface ErrorBoundaryProps {
    children?: React.ReactNode;
    errorComponent: ErrorComponent | undefined;
    errorStyles?: React.ReactNode | undefined;
    errorScripts?: React.ReactNode | undefined;
}
interface ErrorBoundaryHandlerProps extends ErrorBoundaryProps {
    pathname: string | null;
    errorComponent: ErrorComponent;
}
interface ErrorBoundaryHandlerState {
    error: Error | null;
    previousPathname: string | null;
}
export declare class ErrorBoundaryHandler extends React.Component<ErrorBoundaryHandlerProps, ErrorBoundaryHandlerState> {
    constructor(props: ErrorBoundaryHandlerProps);
    static getDerivedStateFromError(error: Error): {
        error: Error;
    };
    static getDerivedStateFromProps(props: ErrorBoundaryHandlerProps, state: ErrorBoundaryHandlerState): ErrorBoundaryHandlerState | null;
    reset: () => void;
    render(): React.ReactNode;
}
export declare function GlobalError({ error }: {
    error: any;
}): import("react/jsx-runtime").JSX.Element;
export default GlobalError;
/**
 * Handles errors through `getDerivedStateFromError`.
 * Renders the provided error component and provides a way to `reset` the error boundary state.
 */
/**
 * Renders error boundary with the provided "errorComponent" property as the fallback.
 * If no "errorComponent" property is provided it renders the children without an error boundary.
 */
export declare function ErrorBoundary({ errorComponent, errorStyles, errorScripts, children, }: ErrorBoundaryProps & {
    children: React.ReactNode;
}): JSX.Element;
