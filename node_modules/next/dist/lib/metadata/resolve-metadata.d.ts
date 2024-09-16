/// <reference types="node" />
import type { Metadata, ResolvedMetadata, ResolvedViewport, ResolvingMetadata, ResolvingViewport, Viewport } from './types/metadata-interface';
import type { GetDynamicParamFromSegment } from '../../server/app-render/app-render';
import type { MetadataContext } from './types/resolvers';
import type { LoaderTree } from '../../server/lib/app-dir-module';
import type { ParsedUrlQuery } from 'querystring';
import type { StaticMetadata } from './types/icons';
type MetadataResolver = (parent: ResolvingMetadata) => Metadata | Promise<Metadata>;
type ViewportResolver = (parent: ResolvingViewport) => Viewport | Promise<Viewport>;
export type MetadataItems = [
    Metadata | MetadataResolver | null,
    StaticMetadata,
    Viewport | ViewportResolver | null
][];
export declare function collectMetadata({ tree, metadataItems, errorMetadataItem, props, route, errorConvention, }: {
    tree: LoaderTree;
    metadataItems: MetadataItems;
    errorMetadataItem: MetadataItems[number];
    props: any;
    route: string;
    errorConvention?: 'not-found';
}): Promise<void>;
export declare function resolveMetadataItems({ tree, parentParams, metadataItems, errorMetadataItem, treePrefix, getDynamicParamFromSegment, searchParams, errorConvention, }: {
    tree: LoaderTree;
    parentParams: {
        [key: string]: any;
    };
    metadataItems: MetadataItems;
    errorMetadataItem: MetadataItems[number];
    /** Provided tree can be nested subtree, this argument says what is the path of such subtree */
    treePrefix?: string[];
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    searchParams: ParsedUrlQuery;
    errorConvention: 'not-found' | undefined;
}): Promise<MetadataItems>;
export declare function accumulateMetadata(metadataItems: MetadataItems, metadataContext: MetadataContext): Promise<ResolvedMetadata>;
export declare function accumulateViewport(metadataItems: MetadataItems): Promise<ResolvedViewport>;
export declare function resolveMetadata({ tree, parentParams, metadataItems, errorMetadataItem, getDynamicParamFromSegment, searchParams, errorConvention, metadataContext, }: {
    tree: LoaderTree;
    parentParams: {
        [key: string]: any;
    };
    metadataItems: MetadataItems;
    errorMetadataItem: MetadataItems[number];
    /** Provided tree can be nested subtree, this argument says what is the path of such subtree */
    treePrefix?: string[];
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    searchParams: {
        [key: string]: any;
    };
    errorConvention: 'not-found' | undefined;
    metadataContext: MetadataContext;
}): Promise<[any, ResolvedMetadata, ResolvedViewport]>;
export {};
