import { type FetchServerResponseResult } from './fetch-server-response';
import { PrefetchCacheEntryStatus, type PrefetchCacheEntry, PrefetchKind, type ReadonlyReducerState } from './router-reducer-types';
/**
 * Returns a prefetch cache entry if one exists. Otherwise creates a new one and enqueues a fetch request
 * to retrieve the prefetch data from the server.
 */
export declare function getOrCreatePrefetchCacheEntry({ url, nextUrl, tree, buildId, prefetchCache, kind, }: Pick<ReadonlyReducerState, 'nextUrl' | 'prefetchCache' | 'tree' | 'buildId'> & {
    url: URL;
    kind?: PrefetchKind;
}): PrefetchCacheEntry;
/**
 * Use to seed the prefetch cache with data that has already been fetched.
 */
export declare function createPrefetchCacheEntryForInitialLoad({ nextUrl, tree, prefetchCache, url, kind, data, }: Pick<ReadonlyReducerState, 'nextUrl' | 'tree' | 'prefetchCache'> & {
    url: URL;
    kind: PrefetchKind;
    data: FetchServerResponseResult;
}): {
    treeAtTimeOfPrefetch: import("../../../server/app-render/types").FlightRouterState;
    data: Promise<FetchServerResponseResult>;
    kind: PrefetchKind;
    prefetchTime: number;
    lastUsedTime: number;
    key: string;
    status: PrefetchCacheEntryStatus;
};
export declare function prunePrefetchCache(prefetchCache: ReadonlyReducerState['prefetchCache']): void;
