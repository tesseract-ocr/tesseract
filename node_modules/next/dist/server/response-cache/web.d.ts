import type { ResponseCacheEntry, ResponseGenerator } from './types';
/**
 * In the web server, there is currently no incremental cache provided and we
 * always SSR the page.
 */
export default class WebResponseCache {
    pendingResponses: Map<string, Promise<ResponseCacheEntry | null>>;
    previousCacheItem?: {
        key: string;
        entry: ResponseCacheEntry | null;
        expiresAt: number;
    };
    minimalMode?: boolean;
    constructor(minimalMode: boolean);
    get(key: string | null, responseGenerator: ResponseGenerator, context: {
        isOnDemandRevalidate?: boolean;
        isPrefetch?: boolean;
        incrementalCache: any;
    }): Promise<ResponseCacheEntry | null>;
}
