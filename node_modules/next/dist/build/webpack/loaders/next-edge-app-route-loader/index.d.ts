import type { webpack } from 'next/dist/compiled/webpack/webpack';
export type EdgeAppRouteLoaderQuery = {
    absolutePagePath: string;
    page: string;
    appDirLoader: string;
    preferredRegion: string | string[] | undefined;
    nextConfig: string;
    middlewareConfig: string;
};
declare const EdgeAppRouteLoader: webpack.LoaderDefinitionFunction<EdgeAppRouteLoaderQuery>;
export default EdgeAppRouteLoader;
