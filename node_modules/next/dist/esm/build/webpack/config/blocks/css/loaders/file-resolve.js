export function cssFileResolve(url, _resourcePath, urlImports) {
    if (url.startsWith("/")) {
        return false;
    }
    if (!urlImports && /^[a-z][a-z0-9+.-]*:/i.test(url)) {
        return false;
    }
    return true;
}

//# sourceMappingURL=file-resolve.js.map