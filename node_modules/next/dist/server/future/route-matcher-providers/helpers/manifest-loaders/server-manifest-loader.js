"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ServerManifestLoader", {
    enumerable: true,
    get: function() {
        return ServerManifestLoader;
    }
});
class ServerManifestLoader {
    constructor(getter){
        this.getter = getter;
    }
    load(name) {
        return this.getter(name);
    }
}

//# sourceMappingURL=server-manifest-loader.js.map