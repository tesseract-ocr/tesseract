import { NodeModuleLoader } from "./node-module-loader";
export class RouteModuleLoader {
    static async load(id, loader = new NodeModuleLoader()) {
        const module = await loader.load(id);
        if ("routeModule" in module) {
            return module.routeModule;
        }
        throw new Error(`Module "${id}" does not export a routeModule.`);
    }
}

//# sourceMappingURL=route-module-loader.js.map