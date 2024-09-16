import type { NextConfigComplete } from '../config-shared';
import '../require-hook';
import '../node-environment';
import type { IncrementalCache } from '../lib/incremental-cache';
type RuntimeConfig = {
    configFileName: string;
    publicRuntimeConfig: {
        [key: string]: any;
    };
    serverRuntimeConfig: {
        [key: string]: any;
    };
};
export declare function loadStaticPaths({ dir, distDir, pathname, config, httpAgentOptions, locales, defaultLocale, isAppPath, page, isrFlushToDisk, fetchCacheKeyPrefix, maxMemoryCacheSize, requestHeaders, cacheHandler, ppr, }: {
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
    ppr: boolean;
}): Promise<{
    paths?: string[];
    encodedPaths?: string[];
    fallback?: boolean | 'blocking';
}>;
export {};
