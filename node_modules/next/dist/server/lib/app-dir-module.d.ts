import type { AppDirModules } from '../../build/webpack/loaders/next-app-loader';
/**
 * LoaderTree is generated in next-app-loader.
 */
export type LoaderTree = [
    segment: string,
    parallelRoutes: {
        [parallelRouterKey: string]: LoaderTree;
    },
    modules: AppDirModules
];
export declare function getLayoutOrPageModule(loaderTree: LoaderTree): Promise<{
    mod: any;
    modType: "page" | "layout" | undefined;
    filePath: string | undefined;
}>;
export declare function getComponentTypeModule(loaderTree: LoaderTree, moduleType: 'layout' | 'not-found' | 'forbidden' | 'unauthorized'): Promise<any>;
