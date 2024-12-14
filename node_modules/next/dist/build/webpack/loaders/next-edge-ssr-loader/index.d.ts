import type webpack from 'webpack';
import type { SizeLimit } from '../../../../types';
import type { PAGE_TYPES } from '../../../../lib/page-types';
export type EdgeSSRLoaderQuery = {
    absolute500Path: string;
    absoluteAppPath: string;
    absoluteDocumentPath: string;
    absoluteErrorPath: string;
    absolutePagePath: string;
    dev: boolean;
    isServerComponent: boolean;
    page: string;
    stringifiedConfig: string;
    appDirLoader?: string;
    pagesType: PAGE_TYPES;
    sriEnabled: boolean;
    cacheHandler?: string;
    cacheHandlers?: string;
    preferredRegion: string | string[] | undefined;
    middlewareConfig: string;
    serverActions?: {
        bodySizeLimit?: SizeLimit;
        allowedOrigins?: string[];
    };
};
declare const edgeSSRLoader: webpack.LoaderDefinitionFunction<EdgeSSRLoaderQuery>;
export default edgeSSRLoader;
