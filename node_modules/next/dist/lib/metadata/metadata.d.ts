import type { ParsedUrlQuery } from 'querystring';
import type { GetDynamicParamFromSegment } from '../../server/app-render/app-render';
import type { LoaderTree } from '../../server/lib/app-dir-module';
import type { CreateServerParamsForMetadata } from '../../server/request/params';
import { type MetadataErrorType } from './resolve-metadata';
import type { MetadataContext } from './types/resolvers';
import type { WorkStore } from '../../server/app-render/work-async-storage.external';
export declare function createMetadataComponents({ tree, searchParams, metadataContext, getDynamicParamFromSegment, appUsingSizeAdjustment, errorType, createServerParamsForMetadata, workStore, MetadataBoundary, ViewportBoundary, }: {
    tree: LoaderTree;
    searchParams: Promise<ParsedUrlQuery>;
    metadataContext: MetadataContext;
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    appUsingSizeAdjustment: boolean;
    errorType?: MetadataErrorType | 'redirect';
    createServerParamsForMetadata: CreateServerParamsForMetadata;
    workStore: WorkStore;
    MetadataBoundary: (props: {
        children: React.ReactNode;
    }) => React.ReactNode;
    ViewportBoundary: (props: {
        children: React.ReactNode;
    }) => React.ReactNode;
}): [React.ComponentType, () => Promise<void>];
