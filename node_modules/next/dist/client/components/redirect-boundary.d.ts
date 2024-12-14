import React from 'react';
import type { AppRouterInstance } from '../../shared/lib/app-router-context.shared-runtime';
import { RedirectType } from './redirect-error';
interface RedirectBoundaryProps {
    router: AppRouterInstance;
    children: React.ReactNode;
}
export declare class RedirectErrorBoundary extends React.Component<RedirectBoundaryProps, {
    redirect: string | null;
    redirectType: RedirectType | null;
}> {
    constructor(props: RedirectBoundaryProps);
    static getDerivedStateFromError(error: any): {
        redirect: string;
        redirectType: RedirectType;
    };
    render(): React.ReactNode;
}
export declare function RedirectBoundary({ children }: {
    children: React.ReactNode;
}): import("react/jsx-runtime").JSX.Element;
export {};
