// Translates a logical route into its pages asset path (relative from a common prefix)
// "asset path" being its javascript file, data file, prerendered html,...
export default function getAssetPathFromRoute(route, ext) {
    if (ext === void 0) ext = '';
    const path = route === '/' ? '/index' : /^\/index(\/|$)/.test(route) ? "/index" + route : route;
    return path + ext;
}

//# sourceMappingURL=get-asset-path-from-route.js.map