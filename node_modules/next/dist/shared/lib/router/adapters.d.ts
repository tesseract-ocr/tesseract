import type { AppRouterInstance } from '../app-router-context.shared-runtime';
import type { Params } from './utils/route-matcher';
import type { NextRouter } from './router';
import React from 'react';
/** It adapts a Pages Router (`NextRouter`) to the App Router Instance. */
export declare function adaptForAppRouterInstance(pagesRouter: NextRouter): AppRouterInstance;
/**
 * adaptForSearchParams transforms the ParsedURLQuery into URLSearchParams.
 *
 * @param router the router that contains the query.
 * @returns the search params in the URLSearchParams format
 */
export declare function adaptForSearchParams(router: Pick<NextRouter, 'isReady' | 'query' | 'asPath'>): URLSearchParams;
export declare function adaptForPathParams(router: Pick<NextRouter, 'isReady' | 'pathname' | 'query' | 'asPath'>): Params | null;
export declare function PathnameContextProviderAdapter({ children, router, ...props }: React.PropsWithChildren<{
    router: Pick<NextRouter, 'pathname' | 'asPath' | 'isReady' | 'isFallback'>;
    isAutoExport: boolean;
}>): import("react/jsx-runtime").JSX.Element;
