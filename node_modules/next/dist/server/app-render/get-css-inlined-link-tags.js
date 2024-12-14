"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getLinkAndScriptTags", {
    enumerable: true,
    get: function() {
        return getLinkAndScriptTags;
    }
});
function getLinkAndScriptTags(clientReferenceManifest, filePath, injectedCSS, injectedScripts, collectNewImports) {
    var _clientReferenceManifest_entryJSFiles;
    const filePathWithoutExt = filePath.replace(/\.[^.]+$/, '');
    const cssChunks = new Set();
    const jsChunks = new Set();
    const entryCSSFiles = clientReferenceManifest.entryCSSFiles[filePathWithoutExt];
    const entryJSFiles = ((_clientReferenceManifest_entryJSFiles = clientReferenceManifest.entryJSFiles) == null ? void 0 : _clientReferenceManifest_entryJSFiles[filePathWithoutExt]) ?? [];
    if (entryCSSFiles) {
        for (const css of entryCSSFiles){
            if (!injectedCSS.has(css.path)) {
                if (collectNewImports) {
                    injectedCSS.add(css.path);
                }
                cssChunks.add(css);
            }
        }
    }
    if (entryJSFiles) {
        for (const file of entryJSFiles){
            if (!injectedScripts.has(file)) {
                if (collectNewImports) {
                    injectedScripts.add(file);
                }
                jsChunks.add(file);
            }
        }
    }
    return {
        styles: [
            ...cssChunks
        ],
        scripts: [
            ...jsChunks
        ]
    };
}

//# sourceMappingURL=get-css-inlined-link-tags.js.map