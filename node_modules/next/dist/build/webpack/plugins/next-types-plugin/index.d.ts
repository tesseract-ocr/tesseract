import type { Rewrite, Redirect } from '../../../../lib/load-custom-routes';
import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { PageExtensions } from '../../../page-extensions-type';
type Rewrites = {
    fallback: Rewrite[];
    afterFiles: Rewrite[];
    beforeFiles: Rewrite[];
};
interface Options {
    dir: string;
    distDir: string;
    appDir: string;
    dev: boolean;
    isEdgeServer: boolean;
    pageExtensions: PageExtensions;
    typedRoutes: boolean;
    originalRewrites: Rewrites | undefined;
    originalRedirects: Redirect[] | undefined;
}
export declare class NextTypesPlugin {
    dir: string;
    distDir: string;
    appDir: string;
    dev: boolean;
    isEdgeServer: boolean;
    pageExtensions: string[];
    pagesDir: string;
    typedRoutes: boolean;
    distDirAbsolutePath: string;
    constructor(options: Options);
    getRelativePathFromAppTypesDir(moduleRelativePathToAppDir: string): string;
    collectPage(filePath: string): void;
    apply(compiler: webpack.Compiler): void;
}
export {};
