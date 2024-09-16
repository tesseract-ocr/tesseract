// Translate a pages asset path (relative from a common prefix) back into its logical route
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, // "asset path" being its javascript file, data file, prerendered html,...
"default", {
    enumerable: true,
    get: function() {
        return getRouteFromAssetPath;
    }
});
const _isdynamic = require("./is-dynamic");
function getRouteFromAssetPath(assetPath, ext) {
    if (ext === void 0) ext = "";
    assetPath = assetPath.replace(/\\/g, "/");
    assetPath = ext && assetPath.endsWith(ext) ? assetPath.slice(0, -ext.length) : assetPath;
    if (assetPath.startsWith("/index/") && !(0, _isdynamic.isDynamicRoute)(assetPath)) {
        assetPath = assetPath.slice(6);
    } else if (assetPath === "/index") {
        assetPath = "/";
    }
    return assetPath;
}

//# sourceMappingURL=get-route-from-asset-path.js.map