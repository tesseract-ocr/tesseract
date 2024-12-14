import type { NextConfigComplete } from '../config-shared';
import '../require-hook';
import '../node-environment';
import type { PartialStaticPathsResult } from '../../build/utils';
import type { IncrementalCache } from '../lib/incremental-cache';
import { type ExperimentalPPRConfig } from '../lib/experimental/ppr';
type RuntimeConfig = {
    pprConfig: ExperimentalPPRConfig | undefined;
    configFileName: string;
    publicRuntimeConfig: {
        [key: string]: any;
    };
    serverRuntimeConfig: {
        [key: string]: any;
    };
    dynamicIO: boolean;
};
export declare function loadStaticPaths({ dir, distDir, pathname, config, httpAgentOptions, locales, defaultLocale, isAppPath, page, isrFlushToDisk, fetchCacheKeyPrefix, maxMemoryCacheSize, requestHeaders, cacheHandler, cacheLifeProfiles, nextConfigOutput, buildId, authInterrupts, }: {
    dir: string;
    distDir: string;
    pathname: string;
    config: RuntimeConfig;
    httpAgentOptions: NextConfigComplete['httpAgentOptions'];
    locales?: string[];
    defaultLocale?: string;
    isAppPath: boolean;
    page: string;
    isrFlushToDisk?: boolean;
    fetchCacheKeyPrefix?: string;
    maxMemoryCacheSize?: number;
    requestHeaders: IncrementalCache['requestHeaders'];
    cacheHandler?: string;
    cacheLifeProfiles?: {
        [profile: string]: import('../../server/use-cache/cache-life').CacheLife;
    };
    nextConfigOutput: 'standalone' | 'export' | undefined;
    buildId: string;
    authInterrupts: boolean;
}): Promise<PartialStaticPathsResult>;
export {};
