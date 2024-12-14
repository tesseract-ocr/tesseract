import type { CacheNodeSeedData, PreloadCallbacks } from './types';
import type { LoaderTree } from '../lib/app-dir-module';
import type { CreateSegmentPath, AppRenderContext } from './app-render';
import type { Params } from '../request/params';
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
    getMetadataReady: () => Promise<void>;
    ctx: AppRenderContext;
    missingSlots?: Set<string>;
    preloadCallbacks: PreloadCallbacks;
    authInterrupts: boolean;
}): Promise<CacheNodeSeedData>;
