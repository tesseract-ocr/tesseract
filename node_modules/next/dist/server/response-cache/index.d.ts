import type { IncrementalCache, ResponseCacheEntry, ResponseGenerator, ResponseCacheBase } from './types';
import { RouteKind } from '../future/route-kind';
export * from './types';
export default class ResponseCache implements ResponseCacheBase {
    private readonly batcher;
    private previousCacheItem?;
    private minimalMode?;
    constructor(minimalMode: boolean);
    get(key: string | null, responseGenerator: ResponseGenerator, context: {
        routeKind?: RouteKind;
        isOnDemandRevalidate?: boolean;
        isPrefetch?: boolean;
        incrementalCache: IncrementalCache;
    }): Promise<ResponseCacheEntry | null>;
}
