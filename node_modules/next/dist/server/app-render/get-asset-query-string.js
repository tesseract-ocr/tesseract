"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getAssetQueryString", {
    enumerable: true,
    get: function() {
        return getAssetQueryString;
    }
});
const isDev = process.env.NODE_ENV === 'development';
const isTurbopack = !!process.env.TURBOPACK;
function getAssetQueryString(ctx, addTimestamp) {
    let qs = '';
    // In development we add the request timestamp to allow react to
    // reload assets when a new RSC response is received.
    // Turbopack handles HMR of assets itself and react doesn't need to reload them
    // so this approach is not needed for Turbopack.
    if (isDev && !isTurbopack && addTimestamp) {
        qs += `?v=${ctx.requestTimestamp}`;
    }
    if (ctx.renderOpts.deploymentId) {
        qs += `${isDev ? '&' : '?'}dpl=${ctx.renderOpts.deploymentId}`;
    }
    return qs;
}

//# sourceMappingURL=get-asset-query-string.js.map