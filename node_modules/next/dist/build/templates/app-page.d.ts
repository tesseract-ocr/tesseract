import type { LoaderTree } from '../../server/lib/app-dir-module';
import { AppPageRouteModule } from '../../server/future/route-modules/app-page/module.compiled';
/**
 * The tree created in next-app-loader that holds component segments and modules
 * and I've updated it.
 */
declare const tree: LoaderTree;
declare const pages: any;
export { tree, pages };
export { default as GlobalError } from 'VAR_MODULE_GLOBAL_ERROR';
export declare const originalPathname = "VAR_ORIGINAL_PATHNAME";
export declare const __next_app__: {
    require: any;
    loadChunk: any;
};
export * from '../../server/app-render/entry-base';
export declare const routeModule: AppPageRouteModule;
