import { type FetchServerResponseResult } from './fetch-server-response';
import { PrefetchCacheEntryStatus, type PrefetchCacheEntry, PrefetchKind, type ReadonlyReducerState } from './router-reducer-types';
export type AliasedPrefetchCacheEntry = PrefetchCacheEntry & {
    /** This is a special property that indicates a prefetch entry associated with a different URL
     * was returned rather than the requested URL. This signals to the router that it should only
     * apply the part that doesn't depend on searchParams (specifically the loading state).
     */
    aliased?: boolean;
};
/**
 * Returns a prefetch cache entry if one exists. Otherwise creates a new one and enqueues a fetch request
 * to retrieve the prefetch data from the server.
 */
export declare function getOrCreatePrefetchCacheEntry({ url, nextUrl, tree, prefetchCache, kind, allowAliasing, }: Pick<ReadonlyReducerState, 'nextUrl' | 'prefetchCache' | 'tree'> & {
    url: URL;
    kind?: PrefetchKind;
    allowAliasing: boolean;
}): AliasedPrefetchCacheEntry;
/**
 * Use to seed the prefetch cache with data that has already been fetched.
 */
export declare function createSeededPrefetchCacheEntry({ nextUrl, tree, prefetchCache, url, data, kind, }: Pick<ReadonlyReducerState, 'nextUrl' | 'tree' | 'prefetchCache'> & {
    url: URL;
    data: FetchServerResponseResult;
    kind: PrefetchKind;
}): {
    treeAtTimeOfPrefetch: import("../../../server/app-render/types").FlightRouterState;
    data: Promise<FetchServerResponseResult>;
    kind: PrefetchKind;
    prefetchTime: number;
    lastUsedTime: number;
    staleTime: number;
    key: string;
    status: PrefetchCacheEntryStatus.fresh;
    url: URL;
};
export declare function prunePrefetchCache(prefetchCache: ReadonlyReducerState['prefetchCache']): void;
