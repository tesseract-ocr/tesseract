import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightRouterState } from '../../../server/app-render/types';
/**
 * Invalidate cache one level down from the router state.
 */
export declare function invalidateCacheByRouterState(newCache: CacheNode, existingCache: CacheNode, routerState: FlightRouterState): void;
