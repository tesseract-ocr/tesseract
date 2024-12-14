import type { CacheNodeSeedData, FlightRouterState } from '../../../server/app-render/types';
import type { CacheNode, ChildSegmentMap } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FetchServerResponseResult } from './fetch-server-response';
export type Task = {
    route: FlightRouterState;
    node: CacheNode | null;
    needsDynamicRequest: boolean;
    children: Map<string, Task> | null;
};
export declare function updateCacheNodeOnNavigation(oldCacheNode: CacheNode, oldRouterState: FlightRouterState, newRouterState: FlightRouterState, prefetchData: CacheNodeSeedData | null, prefetchHead: React.ReactNode | null, isPrefetchHeadPartial: boolean): Task | null;
export declare function listenForDynamicRequest(task: Task, responsePromise: Promise<FetchServerResponseResult>): void;
export declare function abortTask(task: Task, error: any): void;
export declare function updateCacheNodeOnPopstateRestoration(oldCacheNode: CacheNode, routerState: FlightRouterState): {
    lazyData: null;
    rsc: import("react").ReactNode;
    head: import("react").ReactNode;
    prefetchHead: import("react").ReactNode;
    prefetchRsc: import("react").ReactNode;
    loading: import("../../../shared/lib/app-router-context.shared-runtime").LoadingModuleData | Promise<import("../../../shared/lib/app-router-context.shared-runtime").LoadingModuleData>;
    parallelRoutes: Map<string, ChildSegmentMap>;
};
