import type { I18NDomains, NextConfigComplete } from '../../../server/config-shared';
import type { MiddlewareMatcher } from '../../analysis/get-page-static-info';
import { webpack } from 'next/dist/compiled/webpack/webpack';
type BloomFilter = ReturnType<import('../../../shared/lib/bloom-filter').BloomFilter['export']>;
export interface DefineEnvPluginOptions {
    isTurbopack: boolean;
    clientRouterFilters?: {
        staticFilter: BloomFilter;
        dynamicFilter: BloomFilter;
    };
    config: NextConfigComplete;
    dev: boolean;
    distDir: string;
    fetchCacheKeyPrefix: string | undefined;
    hasRewrites: boolean;
    isClient: boolean;
    isEdgeServer: boolean;
    isNodeOrEdgeCompilation: boolean;
    isNodeServer: boolean;
    middlewareMatchers: MiddlewareMatcher[] | undefined;
}
interface DefineEnv {
    [key: string]: string | string[] | boolean | MiddlewareMatcher[] | BloomFilter | Partial<NextConfigComplete['images']> | I18NDomains;
}
interface SerializedDefineEnv {
    [key: string]: string;
}
/**
 * Collects all environment variables that are using the `NEXT_PUBLIC_` prefix.
 */
export declare function getNextPublicEnvironmentVariables(): DefineEnv;
/**
 * Collects the `env` config value from the Next.js config.
 */
export declare function getNextConfigEnv(config: NextConfigComplete): DefineEnv;
export declare function getDefineEnv({ isTurbopack, clientRouterFilters, config, dev, distDir, fetchCacheKeyPrefix, hasRewrites, isClient, isEdgeServer, isNodeOrEdgeCompilation, isNodeServer, middlewareMatchers, }: DefineEnvPluginOptions): SerializedDefineEnv;
export declare function getDefineEnvPlugin(options: DefineEnvPluginOptions): webpack.DefinePlugin;
export {};
