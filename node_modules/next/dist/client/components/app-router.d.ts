import type { ReactNode } from 'react';
import type { CacheNode } from '../../shared/lib/app-router-context.shared-runtime';
import type { ErrorComponent } from './error-boundary';
import type { ServerActionDispatcher } from './router-reducer/router-reducer-types';
import type { InitialRouterStateParameters } from './router-reducer/create-initial-router-state';
export declare function getServerActionDispatcher(): ServerActionDispatcher | null;
export declare function urlToUrlWithoutFlightMarker(url: string): URL;
type AppRouterProps = Omit<Omit<InitialRouterStateParameters, 'isServer' | 'location'>, 'initialParallelRoutes'> & {
    buildId: string;
    initialHead: ReactNode;
    assetPrefix: string;
    missingSlots: Set<string>;
};
export declare function createEmptyCacheNode(): CacheNode;
export default function AppRouter(props: AppRouterProps & {
    globalErrorComponent: ErrorComponent;
}): import("react/jsx-runtime").JSX.Element;
export {};
