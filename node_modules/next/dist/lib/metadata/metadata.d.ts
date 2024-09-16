/// <reference types="node" />
import type { ParsedUrlQuery } from 'querystring';
import type { AppRenderContext, GetDynamicParamFromSegment } from '../../server/app-render/app-render';
import type { LoaderTree } from '../../server/lib/app-dir-module';
import React from 'react';
import type { MetadataContext } from './types/resolvers';
export declare function createMetadataContext(urlPathname: string, renderOpts: AppRenderContext['renderOpts']): MetadataContext;
export declare function createMetadataComponents({ tree, query, metadataContext, getDynamicParamFromSegment, appUsingSizeAdjustment, errorType, createDynamicallyTrackedSearchParams, }: {
    tree: LoaderTree;
    query: ParsedUrlQuery;
    metadataContext: MetadataContext;
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    appUsingSizeAdjustment: boolean;
    errorType?: 'not-found' | 'redirect';
    createDynamicallyTrackedSearchParams: (searchParams: ParsedUrlQuery) => ParsedUrlQuery;
}): [React.ComponentType, React.ComponentType];
