import type { FlightRouterState } from '../../../../server/app-render/types';
import type { CacheNode } from '../../../../shared/lib/app-router-context.shared-runtime';
export declare function findHeadInCache(cache: CacheNode, parallelRoutes: FlightRouterState[1]): [CacheNode, string] | null;
