"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ComponentMod: null,
    default: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ComponentMod: function() {
        return ComponentMod;
    },
    default: function() {
        return nHandler;
    }
});
require("../../server/web/globals");
const _adapter = require("../../server/web/adapter");
const _render = require("../webpack/loaders/next-edge-ssr-loader/render");
const _incrementalcache = require("../../server/lib/incremental-cache");
const _apprender = require("../../server/app-render/app-render");
const _VAR_USERLAND = /*#__PURE__*/ _interop_require_wildcard(require("VAR_USERLAND"));
const _pagetypes = require("../../lib/page-types");
const _encryptionutils = require("../../server/app-render/encryption-utils");
const _actionutils = require("../../server/app-render/action-utils");
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
var _self___RSC_MANIFEST;
// OPTIONAL_IMPORT:incrementalCacheHandler
const cacheHandlers = {};
if (!globalThis.__nextCacheHandlers) {
    ;
    globalThis.__nextCacheHandlers = cacheHandlers;
}
const Document = null;
const appMod = null;
const errorMod = null;
const error500Mod = null;
// INJECT:sriEnabled
// INJECT:isServerComponent
// INJECT:dev
// INJECT:serverActions
// INJECT:nextConfig
const maybeJSONParse = (str)=>str ? JSON.parse(str) : undefined;
const buildManifest = self.__BUILD_MANIFEST;
const reactLoadableManifest = maybeJSONParse(self.__REACT_LOADABLE_MANIFEST);
const rscManifest = (_self___RSC_MANIFEST = self.__RSC_MANIFEST) == null ? void 0 : _self___RSC_MANIFEST['VAR_PAGE'];
const rscServerManifest = maybeJSONParse(self.__RSC_SERVER_MANIFEST);
const subresourceIntegrityManifest = sriEnabled ? maybeJSONParse(self.__SUBRESOURCE_INTEGRITY_MANIFEST) : undefined;
const nextFontManifest = maybeJSONParse(self.__NEXT_FONT_MANIFEST);
const interceptionRouteRewrites = maybeJSONParse(self.__INTERCEPTION_ROUTE_REWRITE_MANIFEST) ?? [];
if (rscManifest && rscServerManifest) {
    (0, _encryptionutils.setReferenceManifestsSingleton)({
        page: 'VAR_PAGE',
        clientReferenceManifest: rscManifest,
        serverActionsManifest: rscServerManifest,
        serverModuleMap: (0, _actionutils.createServerModuleMap)({
            serverActionsManifest: rscServerManifest
        })
    });
}
const render = (0, _render.getRender)({
    pagesType: _pagetypes.PAGE_TYPES.APP,
    dev,
    page: 'VAR_PAGE',
    appMod,
    pageMod: _VAR_USERLAND,
    errorMod,
    error500Mod,
    Document,
    buildManifest,
    renderToHTML: _apprender.renderToHTMLOrFlight,
    reactLoadableManifest,
    clientReferenceManifest: isServerComponent ? rscManifest : null,
    serverActionsManifest: isServerComponent ? rscServerManifest : null,
    serverActions: isServerComponent ? serverActions : undefined,
    subresourceIntegrityManifest,
    config: nextConfig,
    buildId: process.env.__NEXT_BUILD_ID,
    nextFontManifest,
    incrementalCacheHandler,
    interceptionRouteRewrites
});
const ComponentMod = _VAR_USERLAND;
function nHandler(opts) {
    return (0, _adapter.adapter)({
        ...opts,
        IncrementalCache: _incrementalcache.IncrementalCache,
        handler: render
    });
}

//# sourceMappingURL=edge-ssr-app.js.map