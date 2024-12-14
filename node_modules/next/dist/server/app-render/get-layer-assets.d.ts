import React from 'react';
import type { AppRenderContext } from './app-render';
import type { PreloadCallbacks } from './types';
export declare function getLayerAssets({ ctx, layoutOrPagePath, injectedCSS: injectedCSSWithCurrentLayout, injectedJS: injectedJSWithCurrentLayout, injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout, preloadCallbacks, }: {
    layoutOrPagePath: string | undefined;
    injectedCSS: Set<string>;
    injectedJS: Set<string>;
    injectedFontPreloadTags: Set<string>;
    ctx: AppRenderContext;
    preloadCallbacks: PreloadCallbacks;
}): React.ReactNode;
