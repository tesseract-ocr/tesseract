/**
 * Find all font files in the CSS response and determine which files should be preloaded.
 * In Google Fonts responses, the @font-face's subset is above it in a comment.
 * Walk through the CSS from top to bottom, keeping track of the current subset.
 */
export declare function findFontFilesInCss(css: string, subsetsToPreload?: string[]): {
    googleFontFileUrl: string;
    preloadFontFile: boolean;
}[];
