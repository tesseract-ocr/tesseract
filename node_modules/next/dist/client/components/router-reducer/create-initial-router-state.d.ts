import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightDataPath } from '../../../server/app-render/types';
import { type PrefetchCacheEntry } from './router-reducer-types';
export interface InitialRouterStateParameters {
    initialCanonicalUrlParts: string[];
    initialParallelRoutes: CacheNode['parallelRoutes'];
    initialFlightData: FlightDataPath[];
    location: Location | null;
    couldBeIntercepted: boolean;
    postponed: boolean;
    prerendered: boolean;
}
export declare function createInitialRouterState({ initialFlightData, initialCanonicalUrlParts, initialParallelRoutes, location, couldBeIntercepted, postponed, prerendered, }: InitialRouterStateParameters): {
    tree: import("../../../server/app-render/types").FlightRouterState;
    cache: import("../../../shared/lib/app-router-context.shared-runtime").ReadyCacheNode;
    prefetchCache: Map<string, PrefetchCacheEntry>;
    pushRef: {
        pendingPush: boolean;
        mpaNavigation: boolean;
        preserveCustomHistoryState: boolean;
    };
    focusAndScrollRef: {
        apply: boolean;
        onlyHashChange: boolean;
        hashFragment: null;
        segmentPaths: never[];
    };
    canonicalUrl: string;
    nextUrl: string | null;
};
