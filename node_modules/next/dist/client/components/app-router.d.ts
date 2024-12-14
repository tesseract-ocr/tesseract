import React from 'react';
import type { CacheNode } from '../../shared/lib/app-router-context.shared-runtime';
import { type ErrorComponent } from './error-boundary';
import type { AppRouterActionQueue } from '../../shared/lib/router/action-queue';
/**
 * Given a link href, constructs the URL that should be prefetched. Returns null
 * in cases where prefetching should be disabled, like external URLs, or
 * during development.
 * @param href The href passed to <Link>, router.prefetch(), or similar
 * @returns A URL object to prefetch, or null if prefetching should be disabled
 */
export declare function createPrefetchURL(href: string): URL | null;
export declare function createEmptyCacheNode(): CacheNode;
export default function AppRouter({ actionQueue, globalErrorComponentAndStyles: [globalErrorComponent, globalErrorStyles], assetPrefix, }: {
    actionQueue: AppRouterActionQueue;
    globalErrorComponentAndStyles: [ErrorComponent, React.ReactNode | undefined];
    assetPrefix: string;
}): import("react/jsx-runtime").JSX.Element;
