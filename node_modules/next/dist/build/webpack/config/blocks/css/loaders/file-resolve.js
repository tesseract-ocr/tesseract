"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "cssFileResolve", {
    enumerable: true,
    get: function() {
        return cssFileResolve;
    }
});
function cssFileResolve(url, _resourcePath, urlImports) {
    if (url.startsWith('/')) {
        return false;
    }
    if (!urlImports && /^[a-z][a-z0-9+.-]*:/i.test(url)) {
        return false;
    }
    return true;
}

//# sourceMappingURL=file-resolve.js.map