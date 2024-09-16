import type { NextFontManifest } from '../../build/webpack/plugins/next-font-manifest-plugin';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
/**
 * Get hrefs for fonts to preload
 * Returns null if there are no fonts at all.
 * Returns string[] if there are fonts to preload (font paths)
 * Returns empty string[] if there are fonts but none to preload and no other fonts have been preloaded
 * Returns null if there are fonts but none to preload and at least some were previously preloaded
 */
export declare function getPreloadableFonts(nextFontManifest: DeepReadonly<NextFontManifest> | undefined, filePath: string | undefined, injectedFontPreloadTags: Set<string>): string[] | null;
