"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getPreloadableFonts", {
    enumerable: true,
    get: function() {
        return getPreloadableFonts;
    }
});
function getPreloadableFonts(nextFontManifest, filePath, injectedFontPreloadTags) {
    if (!nextFontManifest || !filePath) {
        return null;
    }
    const filepathWithoutExtension = filePath.replace(/\.[^.]+$/, "");
    const fontFiles = new Set();
    let foundFontUsage = false;
    const preloadedFontFiles = nextFontManifest.app[filepathWithoutExtension];
    if (preloadedFontFiles) {
        foundFontUsage = true;
        for (const fontFile of preloadedFontFiles){
            if (!injectedFontPreloadTags.has(fontFile)) {
                fontFiles.add(fontFile);
                injectedFontPreloadTags.add(fontFile);
            }
        }
    }
    if (fontFiles.size) {
        return [
            ...fontFiles
        ].sort();
    } else if (foundFontUsage && injectedFontPreloadTags.size === 0) {
        return [];
    } else {
        return null;
    }
}

//# sourceMappingURL=get-preloadable-fonts.js.map