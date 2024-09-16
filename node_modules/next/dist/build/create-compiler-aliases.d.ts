import { type WebpackLayerName } from '../lib/constants';
import type { NextConfigComplete } from '../server/config-shared';
interface CompilerAliases {
    [alias: string]: string | string[];
}
export declare function createWebpackAliases({ distDir, isClient, isEdgeServer, isNodeServer, dev, config, pagesDir, appDir, dir, reactProductionProfiling, hasRewrites, }: {
    distDir: string;
    isClient: boolean;
    isEdgeServer: boolean;
    isNodeServer: boolean;
    dev: boolean;
    config: NextConfigComplete;
    pagesDir: string | undefined;
    appDir: string | undefined;
    dir: string;
    reactProductionProfiling: boolean;
    hasRewrites: boolean;
}): CompilerAliases;
export declare function createServerOnlyClientOnlyAliases(isServer: boolean): CompilerAliases;
export declare function createNextApiEsmAliases(): Record<string, string>;
export declare function createAppRouterApiAliases(isServerOnlyLayer: boolean): Record<string, string>;
export declare function createRSCAliases(bundledReactChannel: string, { layer, isEdgeServer, reactProductionProfiling, }: {
    layer: WebpackLayerName;
    isEdgeServer: boolean;
    reactProductionProfiling: boolean;
}): CompilerAliases;
export declare function getOptimizedModuleAliases(): CompilerAliases;
export {};
