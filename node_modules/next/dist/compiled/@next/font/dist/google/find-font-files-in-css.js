"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.findFontFilesInCss = void 0;
/**
 * Find all font files in the CSS response and determine which files should be preloaded.
 * In Google Fonts responses, the @font-face's subset is above it in a comment.
 * Walk through the CSS from top to bottom, keeping track of the current subset.
 */
function findFontFilesInCss(css, subsetsToPreload) {
    var _a, _b;
    // Find font files to download
    const fontFiles = [];
    // Keep track of the current subset
    let currentSubset = '';
    for (const line of css.split('\n')) {
        const newSubset = (_a = /\/\* (.+?) \*\//.exec(line)) === null || _a === void 0 ? void 0 : _a[1];
        if (newSubset) {
            // Found new subset in a comment above the next @font-face declaration
            currentSubset = newSubset;
        }
        else {
            const googleFontFileUrl = (_b = /src: url\((.+?)\)/.exec(line)) === null || _b === void 0 ? void 0 : _b[1];
            if (googleFontFileUrl &&
                !fontFiles.some((foundFile) => foundFile.googleFontFileUrl === googleFontFileUrl)) {
                // Found the font file in the @font-face declaration.
                fontFiles.push({
                    googleFontFileUrl,
                    preloadFontFile: !!(subsetsToPreload === null || subsetsToPreload === void 0 ? void 0 : subsetsToPreload.includes(currentSubset)),
                });
            }
        }
    }
    return fontFiles;
}
exports.findFontFilesInCss = findFontFilesInCss;
