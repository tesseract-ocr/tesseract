import type { CollectingMetadata } from './types';
import type { MetadataResolver } from '../next-app-loader';
import type { PageExtensions } from '../../../page-extensions-type';
export declare function createStaticMetadataFromRoute(resolvedDir: string, { segment, metadataResolver, isRootLayoutOrRootPage, pageExtensions, basePath, }: {
    segment: string;
    metadataResolver: MetadataResolver;
    isRootLayoutOrRootPage: boolean;
    pageExtensions: PageExtensions;
    basePath: string;
}): Promise<CollectingMetadata | null>;
export declare function createMetadataExportsCode(metadata: Awaited<ReturnType<typeof createStaticMetadataFromRoute>>): string;
