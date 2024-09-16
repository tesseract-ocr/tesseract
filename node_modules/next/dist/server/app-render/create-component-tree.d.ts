import type { CacheNodeSeedData } from './types';
import React from 'react';
import type { LoaderTree } from '../lib/app-dir-module';
import type { CreateSegmentPath, AppRenderContext } from './app-render';
type Params = {
    [key: string]: string | string[];
};
/**
 * Use the provided loader tree to create the React Component tree.
 */
export declare function createComponentTree(props: {
    createSegmentPath: CreateSegmentPath;
    loaderTree: LoaderTree;
    parentParams: Params;
    rootLayoutIncluded: boolean;
    firstItem?: boolean;
    injectedCSS: Set<string>;
    injectedJS: Set<string>;
    injectedFontPreloadTags: Set<string>;
    asNotFound?: boolean;
    metadataOutlet?: React.ReactNode;
    ctx: AppRenderContext;
    missingSlots?: Set<string>;
}): Promise<CacheNodeSeedData>;
export {};
