"use client";

"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PreloadCss", {
    enumerable: true,
    get: function() {
        return PreloadCss;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _requestasyncstorageexternal = require("../../../client/components/request-async-storage.external");
function PreloadCss(param) {
    let { moduleIds } = param;
    // Early return in client compilation and only load requestStore on server side
    if (typeof window !== "undefined") {
        return null;
    }
    const requestStore = (0, _requestasyncstorageexternal.getExpectedRequestStore)("next/dynamic css");
    const allFiles = [];
    // Search the current dynamic call unique key id in react loadable manifest,
    // and find the corresponding CSS files to preload
    if (requestStore.reactLoadableManifest && moduleIds) {
        const manifest = requestStore.reactLoadableManifest;
        for (const key of moduleIds){
            if (!manifest[key]) continue;
            const cssFiles = manifest[key].files.filter((file)=>file.endsWith(".css"));
            allFiles.push(...cssFiles);
        }
    }
    if (allFiles.length === 0) {
        return null;
    }
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: allFiles.map((file)=>{
            return /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                // @ts-ignore
                precedence: "dynamic",
                rel: "stylesheet",
                href: requestStore.assetPrefix + "/_next/" + encodeURI(file),
                as: "style"
            }, file);
        })
    });
}

//# sourceMappingURL=preload-css.js.map