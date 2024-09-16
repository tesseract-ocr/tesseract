import { webpack } from 'next/dist/compiled/webpack/webpack';
export type PagesManifest = {
    [page: string]: string;
};
export declare let edgeServerPages: {};
export declare let nodeServerPages: {};
export declare let edgeServerAppPaths: {};
export declare let nodeServerAppPaths: {};
export default class PagesManifestPlugin implements webpack.WebpackPluginInstance {
    dev: boolean;
    distDir?: string;
    isEdgeRuntime: boolean;
    appDirEnabled: boolean;
    constructor({ dev, distDir, isEdgeRuntime, appDirEnabled, }: {
        dev: boolean;
        distDir?: string;
        isEdgeRuntime: boolean;
        appDirEnabled: boolean;
    });
    createAssets(compilation: any, assets: any): Promise<void>;
    apply(compiler: webpack.Compiler): void;
}
