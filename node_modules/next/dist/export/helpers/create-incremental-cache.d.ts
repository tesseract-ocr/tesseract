import type { NextEnabledDirectories } from '../../server/base-server';
import { IncrementalCache } from '../../server/lib/incremental-cache';
export declare function createIncrementalCache({ cacheHandler, cacheMaxMemorySize, fetchCacheKeyPrefix, distDir, dir, enabledDirectories, experimental, flushToDisk, }: {
    cacheHandler?: string;
    cacheMaxMemorySize?: number;
    fetchCacheKeyPrefix?: string;
    distDir: string;
    dir: string;
    enabledDirectories: NextEnabledDirectories;
    experimental: {
        ppr: boolean;
    };
    flushToDisk?: boolean;
}): Promise<IncrementalCache>;
