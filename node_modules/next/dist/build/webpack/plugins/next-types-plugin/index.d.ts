import type { Rewrite, Redirect } from '../../../../lib/load-custom-routes';
import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { PageExtensions } from '../../../page-extensions-type';
import type { CacheLife } from '../../../../server/use-cache/cache-life';
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
    cacheLifeConfig: undefined | {
        [profile: string]: CacheLife;
    };
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
    cacheLifeConfig: undefined | {
        [profile: string]: CacheLife;
    };
    distDirAbsolutePath: string;
    constructor(options: Options);
    getRelativePathFromAppTypesDir(moduleRelativePathToAppDir: string): string;
    collectPage(filePath: string): void;
    apply(compiler: webpack.Compiler): void;
}
export {};
