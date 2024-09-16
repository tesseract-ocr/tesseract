import type webpack from 'webpack';
export type EdgeFunctionLoaderOptions = {
    absolutePagePath: string;
    page: string;
    rootDir: string;
    preferredRegion: string | string[] | undefined;
    middlewareConfig: string;
};
declare const nextEdgeFunctionLoader: webpack.LoaderDefinitionFunction<EdgeFunctionLoaderOptions>;
export default nextEdgeFunctionLoader;
