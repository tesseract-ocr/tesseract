import type webpack from 'webpack';
type MetadataRouteLoaderOptions = {
    page: string;
    filePath: string;
    isDynamic: '1' | '0';
};
export declare function getFilenameAndExtension(resourcePath: string): {
    name: string;
    ext: string;
};
declare const nextMetadataRouterLoader: webpack.LoaderDefinitionFunction<MetadataRouteLoaderOptions>;
export default nextMetadataRouterLoader;
