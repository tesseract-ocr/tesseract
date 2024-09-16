import type { ComponentsType } from '../../build/webpack/loaders/next-app-loader';
/**
 * LoaderTree is generated in next-app-loader.
 */
export type LoaderTree = [
    segment: string,
    parallelRoutes: {
        [parallelRouterKey: string]: LoaderTree;
    },
    components: ComponentsType
];
export declare function getLayoutOrPageModule(loaderTree: LoaderTree): Promise<readonly [any, "page" | "layout" | undefined]>;
export declare function getComponentTypeModule(loaderTree: LoaderTree, componentType: 'layout' | 'not-found'): Promise<any>;
