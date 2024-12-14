import { IncrementalCache } from '../../server/lib/incremental-cache';
export declare function createIncrementalCache({ cacheHandler, dynamicIO, cacheMaxMemorySize, fetchCacheKeyPrefix, distDir, dir, flushToDisk, cacheHandlers, }: {
    dynamicIO: boolean;
    cacheHandler?: string;
    cacheMaxMemorySize?: number;
    fetchCacheKeyPrefix?: string;
    distDir: string;
    dir: string;
    flushToDisk?: boolean;
    cacheHandlers?: Record<string, string | undefined>;
}): Promise<IncrementalCache>;
