import type webpack from 'webpack';
import type { PossibleImageFileNameConvention } from './metadata/types';
import type { PageExtensions } from '../../page-extensions-type';
interface Options {
    segment: string;
    type: PossibleImageFileNameConvention;
    pageExtensions: PageExtensions;
    basePath: string;
}
declare function nextMetadataImageLoader(this: webpack.LoaderContext<Options>, content: Buffer): Promise<string>;
export declare const raw = true;
export default nextMetadataImageLoader;
