import { CachedRouteMatcherProvider } from "./helpers/cached-route-matcher-provider";
export class ManifestRouteMatcherProvider extends CachedRouteMatcherProvider {
    constructor(manifestName, manifestLoader){
        super({
            load: async ()=>manifestLoader.load(manifestName),
            compare: (left, right)=>left === right
        });
    }
}

//# sourceMappingURL=manifest-route-matcher-provider.js.map