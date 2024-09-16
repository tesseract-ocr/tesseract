import type { CustomRoutes } from '../../../lib/load-custom-routes';
import { webpack } from 'next/dist/compiled/webpack/webpack';
export type ClientBuildManifest = {
    [key: string]: string[];
};
export declare const srcEmptySsgManifest = "self.__SSG_MANIFEST=new Set;self.__SSG_MANIFEST_CB&&self.__SSG_MANIFEST_CB()";
export declare function normalizeRewritesForBuildManifest(rewrites: CustomRoutes['rewrites']): CustomRoutes['rewrites'];
export declare function getEntrypointFiles(entrypoint: any): string[];
export default class BuildManifestPlugin {
    private buildId;
    private rewrites;
    private isDevFallback;
    private exportRuntime;
    private appDirEnabled;
    constructor(options: {
        buildId: string;
        rewrites: CustomRoutes['rewrites'];
        isDevFallback?: boolean;
        exportRuntime?: boolean;
        appDirEnabled: boolean;
    });
    createAssets(compiler: any, compilation: any, assets: any): any;
    apply(compiler: webpack.Compiler): void;
}
