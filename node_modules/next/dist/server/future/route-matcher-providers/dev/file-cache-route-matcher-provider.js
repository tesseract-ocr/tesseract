"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "FileCacheRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return FileCacheRouteMatcherProvider;
    }
});
const _cachedroutematcherprovider = require("../helpers/cached-route-matcher-provider");
class FileCacheRouteMatcherProvider extends _cachedroutematcherprovider.CachedRouteMatcherProvider {
    constructor(dir, reader){
        super({
            load: async ()=>reader.read(dir),
            compare: (left, right)=>{
                if (left.length !== right.length) return false;
                // Assuming the file traversal order is deterministic...
                for(let i = 0; i < left.length; i++){
                    if (left[i] !== right[i]) return false;
                }
                return true;
            }
        });
    }
}

//# sourceMappingURL=file-cache-route-matcher-provider.js.map