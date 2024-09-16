import type { LoaderTree } from '../lib/app-dir-module';
export declare function parseLoaderTree(tree: LoaderTree): {
    page: import("../../build/webpack/loaders/metadata/types").ModuleReference | undefined;
    segment: string;
    components: import("../../build/webpack/loaders/next-app-loader").ComponentsType;
    layoutOrPagePath: string | undefined;
    parallelRoutes: {
        [parallelRouterKey: string]: LoaderTree;
    };
};
