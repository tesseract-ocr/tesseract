import * as React from 'react';
type ErrorBoundaryProps = {
    children?: React.ReactNode;
    onError: (error: Error, componentStack: string | null) => void;
    globalOverlay?: boolean;
    isMounted?: boolean;
};
type ErrorBoundaryState = {
    error: Error | null;
};
export declare class ErrorBoundary extends React.PureComponent<ErrorBoundaryProps, ErrorBoundaryState> {
    state: {
        error: null;
    };
    static getDerivedStateFromError(error: Error): {
        error: Error;
    };
    componentDidCatch(error: Error, errorInfo?: {
        componentStack?: string | null;
    }): void;
    render(): React.ReactNode;
}
export {};
