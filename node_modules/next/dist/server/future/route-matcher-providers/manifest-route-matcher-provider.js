"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ManifestRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return ManifestRouteMatcherProvider;
    }
});
const _cachedroutematcherprovider = require("./helpers/cached-route-matcher-provider");
class ManifestRouteMatcherProvider extends _cachedroutematcherprovider.CachedRouteMatcherProvider {
    constructor(manifestName, manifestLoader){
        super({
            load: async ()=>manifestLoader.load(manifestName),
            compare: (left, right)=>left === right
        });
    }
}

//# sourceMappingURL=manifest-route-matcher-provider.js.map