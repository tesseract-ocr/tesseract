import type { BloomFilter } from '../../../shared/lib/bloom-filter';
import type { Rewrite, CustomRoutes } from '../../../lib/load-custom-routes';
import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { BuildManifest } from '../../../server/get-page-files';
export type ClientBuildManifest = {
    [key: string]: string[];
};
export declare const srcEmptySsgManifest = "self.__SSG_MANIFEST=new Set;self.__SSG_MANIFEST_CB&&self.__SSG_MANIFEST_CB()";
export declare function normalizeRewritesForBuildManifest(rewrites: CustomRoutes['rewrites']): CustomRoutes['rewrites'];
export declare function generateClientManifest(assetMap: BuildManifest, rewrites: CustomRoutes['rewrites'], clientRouterFilters?: {
    staticFilter: ReturnType<BloomFilter['export']>;
    dynamicFilter: ReturnType<BloomFilter['export']>;
}, compiler?: any, compilation?: any): string | undefined;
export declare function getEntrypointFiles(entrypoint: any): string[];
export declare const processRoute: (r: Rewrite) => {
    source: string;
    destination: string;
    basePath?: false;
    locale?: false;
    has?: import("../../../lib/load-custom-routes").RouteHas[];
    missing?: import("../../../lib/load-custom-routes").RouteHas[];
    internal?: boolean;
};
export default class BuildManifestPlugin {
    private buildId;
    private rewrites;
    private isDevFallback;
    private appDirEnabled;
    private clientRouterFilters?;
    constructor(options: {
        buildId: string;
        rewrites: CustomRoutes['rewrites'];
        isDevFallback?: boolean;
        appDirEnabled: boolean;
        clientRouterFilters?: Parameters<typeof generateClientManifest>[2];
    });
    createAssets(compiler: any, compilation: any, assets: any): any;
    apply(compiler: webpack.Compiler): void;
}
