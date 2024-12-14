import type { Metadata, ResolvedMetadata, ResolvedViewport, ResolvingMetadata, ResolvingViewport, Viewport } from './types/metadata-interface';
import type { GetDynamicParamFromSegment } from '../../server/app-render/app-render';
import type { MetadataContext } from './types/resolvers';
import type { LoaderTree } from '../../server/lib/app-dir-module';
import type { ParsedUrlQuery } from 'querystring';
import type { StaticMetadata } from './types/icons';
import 'server-only';
import type { WorkStore } from '../../server/app-render/work-async-storage.external';
import type { CreateServerParamsForMetadata } from '../../server/request/params';
type MetadataResolver = (parent: ResolvingMetadata) => Metadata | Promise<Metadata>;
type ViewportResolver = (parent: ResolvingViewport) => Viewport | Promise<Viewport>;
export type MetadataErrorType = 'not-found' | 'forbidden' | 'unauthorized';
export type MetadataItems = [
    Metadata | MetadataResolver | null,
    StaticMetadata,
    Viewport | ViewportResolver | null
][];
declare const cachedResolveMetadataItems: typeof resolveMetadataItems;
export { cachedResolveMetadataItems as resolveMetadataItems };
declare function resolveMetadataItems(tree: LoaderTree, searchParams: Promise<ParsedUrlQuery>, errorConvention: MetadataErrorType | undefined, getDynamicParamFromSegment: GetDynamicParamFromSegment, createServerParamsForMetadata: CreateServerParamsForMetadata, workStore: WorkStore): Promise<MetadataItems>;
export declare function accumulateMetadata(metadataItems: MetadataItems, metadataContext: MetadataContext): Promise<ResolvedMetadata>;
export declare function accumulateViewport(metadataItems: MetadataItems): Promise<ResolvedViewport>;
