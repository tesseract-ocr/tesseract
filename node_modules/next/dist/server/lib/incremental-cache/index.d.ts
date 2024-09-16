import type { CacheFs } from '../../../shared/lib/utils';
import type { PrerenderManifest } from '../../../build';
import type { IncrementalCacheValue, IncrementalCacheEntry, IncrementalCache as IncrementalCacheType, IncrementalCacheKindHint } from '../../response-cache';
import type { DeepReadonly } from '../../../shared/lib/deep-readonly';
export interface CacheHandlerContext {
    fs?: CacheFs;
    dev?: boolean;
    flushToDisk?: boolean;
    serverDistDir?: string;
    maxMemoryCacheSize?: number;
    fetchCacheKeyPrefix?: string;
    prerenderManifest?: PrerenderManifest;
    revalidatedTags: string[];
    experimental: {
        ppr: boolean;
    };
    _appDir: boolean;
    _pagesDir: boolean;
    _requestHeaders: IncrementalCache['requestHeaders'];
}
export interface CacheHandlerValue {
    lastModified?: number;
    age?: number;
    cacheState?: string;
    value: IncrementalCacheValue | null;
}
export declare class CacheHandler {
    constructor(_ctx: CacheHandlerContext);
    get(..._args: Parameters<IncrementalCache['get']>): Promise<CacheHandlerValue | null>;
    set(..._args: Parameters<IncrementalCache['set']>): Promise<void>;
    revalidateTag(..._args: Parameters<IncrementalCache['revalidateTag']>): Promise<void>;
    resetRequestCache(): void;
}
export declare class IncrementalCache implements IncrementalCacheType {
    readonly dev?: boolean;
    readonly disableForTestmode?: boolean;
    readonly cacheHandler?: CacheHandler;
    readonly hasCustomCacheHandler: boolean;
    readonly prerenderManifest: DeepReadonly<PrerenderManifest>;
    readonly requestHeaders: Record<string, undefined | string | string[]>;
    readonly requestProtocol?: 'http' | 'https';
    readonly allowedRevalidateHeaderKeys?: string[];
    readonly minimalMode?: boolean;
    readonly fetchCacheKeyPrefix?: string;
    readonly revalidatedTags?: string[];
    readonly isOnDemandRevalidate?: boolean;
    private readonly locks;
    private readonly unlocks;
    /**
     * The revalidate timings for routes. This will source the timings from the
     * prerender manifest until the in-memory cache is updated with new timings.
     */
    private readonly revalidateTimings;
    constructor({ fs, dev, appDir, pagesDir, flushToDisk, fetchCache, minimalMode, serverDistDir, requestHeaders, requestProtocol, maxMemoryCacheSize, getPrerenderManifest, fetchCacheKeyPrefix, CurCacheHandler, allowedRevalidateHeaderKeys, experimental, }: {
        fs?: CacheFs;
        dev: boolean;
        appDir?: boolean;
        pagesDir?: boolean;
        fetchCache?: boolean;
        minimalMode?: boolean;
        serverDistDir?: string;
        flushToDisk?: boolean;
        requestProtocol?: 'http' | 'https';
        allowedRevalidateHeaderKeys?: string[];
        requestHeaders: IncrementalCache['requestHeaders'];
        maxMemoryCacheSize?: number;
        getPrerenderManifest: () => DeepReadonly<PrerenderManifest>;
        fetchCacheKeyPrefix?: string;
        CurCacheHandler?: typeof CacheHandler;
        experimental: {
            ppr: boolean;
        };
    });
    private calculateRevalidate;
    _getPathname(pathname: string, fetchCache?: boolean): string;
    resetRequestCache(): void;
    unlock(cacheKey: string): Promise<void>;
    lock(cacheKey: string): Promise<() => Promise<void>>;
    revalidateTag(tags: string | string[]): Promise<void>;
    fetchCacheKey(url: string, init?: RequestInit | Request): Promise<string>;
    get(cacheKey: string, ctx?: {
        kindHint?: IncrementalCacheKindHint;
        revalidate?: number | false;
        fetchUrl?: string;
        fetchIdx?: number;
        tags?: string[];
        softTags?: string[];
    }): Promise<IncrementalCacheEntry | null>;
    set(pathname: string, data: IncrementalCacheValue | null, ctx: {
        revalidate?: number | false;
        fetchCache?: boolean;
        fetchUrl?: string;
        fetchIdx?: number;
        tags?: string[];
    }): Promise<any>;
}
