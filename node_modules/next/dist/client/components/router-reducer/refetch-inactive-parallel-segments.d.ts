import type { FlightRouterState } from '../../../server/app-render/types';
import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { AppRouterState } from './router-reducer-types';
interface RefreshInactiveParallelSegments {
    state: AppRouterState;
    updatedTree: FlightRouterState;
    updatedCache: CacheNode;
    includeNextUrl: boolean;
    canonicalUrl: string;
}
/**
 * Refreshes inactive segments that are still in the current FlightRouterState.
 * A segment is considered "inactive" when the server response indicates it didn't match to a page component.
 * This happens during a soft-navigation, where the server will want to patch in the segment
 * with the "default" component, but we explicitly ignore the server in this case
 * and keep the existing state for that segment. New data for inactive segments are inherently
 * not part of the server response when we patch the tree, because they were associated with a response
 * from an earlier navigation/request. For each segment, once it becomes "active", we encode the URL that provided
 * the data for it. This function traverses parallel routes looking for these markers so that it can re-fetch
 * and patch the new data into the tree.
 */
export declare function refreshInactiveParallelSegments(options: RefreshInactiveParallelSegments): Promise<void>;
/**
 * Walks the current parallel segments to determine if they are "active".
 * An active parallel route will have a `__PAGE__` segment in the FlightRouterState.
 * As opposed to a `__DEFAULT__` segment, which means there was no match for that parallel route.
 * We add a special marker here so that we know how to refresh its data when the router is revalidated.
 */
export declare function addRefreshMarkerToActiveParallelSegments(tree: FlightRouterState, path: string): void;
export {};
