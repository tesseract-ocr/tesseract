import type { CacheHandler, CacheHandlerContext, CacheHandlerValue } from './';
export default class FetchCache implements CacheHandler {
    private headers;
    private cacheEndpoint?;
    private hasMatchingTags;
    static isAvailable(ctx: {
        _requestHeaders: CacheHandlerContext['_requestHeaders'];
    }): boolean;
    constructor(ctx: CacheHandlerContext);
    resetRequestCache(): void;
    revalidateTag(...args: Parameters<CacheHandler['revalidateTag']>): Promise<void>;
    get(...args: Parameters<CacheHandler['get']>): Promise<CacheHandlerValue | null>;
    set(...args: Parameters<CacheHandler['set']>): Promise<void>;
}
