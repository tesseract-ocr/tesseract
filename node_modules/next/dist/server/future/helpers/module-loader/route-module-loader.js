"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "RouteModuleLoader", {
    enumerable: true,
    get: function() {
        return RouteModuleLoader;
    }
});
const _nodemoduleloader = require("./node-module-loader");
class RouteModuleLoader {
    static async load(id, loader = new _nodemoduleloader.NodeModuleLoader()) {
        const module = await loader.load(id);
        if ("routeModule" in module) {
            return module.routeModule;
        }
        throw new Error(`Module "${id}" does not export a routeModule.`);
    }
}

//# sourceMappingURL=route-module-loader.js.map