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
const _VAR_MODULE_DOCUMENT = /*#__PURE__*/ _interop_require_default(require("VAR_MODULE_DOCUMENT"));
const _VAR_MODULE_APP = /*#__PURE__*/ _interop_require_wildcard(require("VAR_MODULE_APP"));
const _VAR_USERLAND = /*#__PURE__*/ _interop_require_wildcard(require("VAR_USERLAND"));
const _VAR_MODULE_GLOBAL_ERROR = /*#__PURE__*/ _interop_require_wildcard(require("VAR_MODULE_GLOBAL_ERROR"));
const _render1 = require("../../server/render");
const _module = /*#__PURE__*/ _interop_require_default(require("../../server/route-modules/pages/module"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
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
// INJECT:pagesType
// INJECT:sriEnabled
// INJECT:dev
// INJECT:nextConfig
// INJECT:pageRouteModuleOptions
// INJECT:errorRouteModuleOptions
// INJECT:user500RouteModuleOptions
const cacheHandlers = {};
if (!globalThis.__nextCacheHandlers) {
    ;
    globalThis.__nextCacheHandlers = cacheHandlers;
}
const pageMod = {
    ..._VAR_USERLAND,
    routeModule: new _module.default({
        ...pageRouteModuleOptions,
        components: {
            App: _VAR_MODULE_APP.default,
            Document: _VAR_MODULE_DOCUMENT.default
        },
        userland: _VAR_USERLAND
    })
};
const errorMod = {
    ..._VAR_MODULE_GLOBAL_ERROR,
    routeModule: new _module.default({
        ...errorRouteModuleOptions,
        components: {
            App: _VAR_MODULE_APP.default,
            Document: _VAR_MODULE_DOCUMENT.default
        },
        userland: _VAR_MODULE_GLOBAL_ERROR
    })
};
// FIXME: this needs to be made compatible with the template
const error500Mod = userland500Page ? {
    ...userland500Page,
    routeModule: new _module.default({
        ...user500RouteModuleOptions,
        components: {
            App: _VAR_MODULE_APP.default,
            Document: _VAR_MODULE_DOCUMENT.default
        },
        userland: userland500Page
    })
} : null;
const maybeJSONParse = (str)=>str ? JSON.parse(str) : undefined;
const buildManifest = self.__BUILD_MANIFEST;
const reactLoadableManifest = maybeJSONParse(self.__REACT_LOADABLE_MANIFEST);
const dynamicCssManifest = maybeJSONParse(self.__DYNAMIC_CSS_MANIFEST);
const subresourceIntegrityManifest = sriEnabled ? maybeJSONParse(self.__SUBRESOURCE_INTEGRITY_MANIFEST) : undefined;
const nextFontManifest = maybeJSONParse(self.__NEXT_FONT_MANIFEST);
const render = (0, _render.getRender)({
    pagesType,
    dev,
    page: 'VAR_PAGE',
    appMod: _VAR_MODULE_APP,
    pageMod,
    errorMod,
    error500Mod,
    Document: _VAR_MODULE_DOCUMENT.default,
    buildManifest,
    renderToHTML: _render1.renderToHTML,
    reactLoadableManifest,
    dynamicCssManifest,
    subresourceIntegrityManifest,
    config: nextConfig,
    buildId: process.env.__NEXT_BUILD_ID,
    nextFontManifest,
    incrementalCacheHandler
});
const ComponentMod = pageMod;
function nHandler(opts) {
    return (0, _adapter.adapter)({
        ...opts,
        IncrementalCache: _incrementalcache.IncrementalCache,
        handler: render
    });
}

//# sourceMappingURL=edge-ssr.js.map