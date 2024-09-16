import type { ReactNode } from 'react';
import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightRouterState, CacheNodeSeedData } from '../../../server/app-render/types';
import { type PrefetchCacheEntry } from './router-reducer-types';
export interface InitialRouterStateParameters {
    buildId: string;
    initialTree: FlightRouterState;
    urlParts: string[];
    initialSeedData: CacheNodeSeedData;
    initialParallelRoutes: CacheNode['parallelRoutes'];
    location: Location | null;
    initialHead: ReactNode;
    couldBeIntercepted?: boolean;
}
export declare function createInitialRouterState({ buildId, initialTree, initialSeedData, urlParts, initialParallelRoutes, location, initialHead, couldBeIntercepted, }: InitialRouterStateParameters): {
    buildId: string;
    tree: FlightRouterState;
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
