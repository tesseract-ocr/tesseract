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
        return _default;
    }
});
const _actionutils = require("../../server/app-render/action-utils");
const _encryptionutils = require("../../server/app-render/encryption-utils");
const _edgeroutemodulewrapper = require("../../server/web/edge-route-module-wrapper");
const _VAR_USERLAND = /*#__PURE__*/ _interop_require_wildcard(require("VAR_USERLAND"));
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
// INJECT:nextConfig
const maybeJSONParse = (str)=>str ? JSON.parse(str) : undefined;
const rscManifest = (_self___RSC_MANIFEST = self.__RSC_MANIFEST) == null ? void 0 : _self___RSC_MANIFEST['VAR_PAGE'];
const rscServerManifest = maybeJSONParse(self.__RSC_SERVER_MANIFEST);
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
const ComponentMod = _VAR_USERLAND;
const _default = _edgeroutemodulewrapper.EdgeRouteModuleWrapper.wrap(_VAR_USERLAND.routeModule, {
    nextConfig
});

//# sourceMappingURL=edge-app-route.js.map