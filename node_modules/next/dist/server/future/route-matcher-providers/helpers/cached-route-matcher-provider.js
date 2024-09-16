"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "CachedRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return CachedRouteMatcherProvider;
    }
});
class CachedRouteMatcherProvider {
    constructor(loader){
        this.loader = loader;
        this.cached = [];
    }
    async matchers() {
        const data = await this.loader.load();
        if (!data) return [];
        // Return the cached matchers if the data has not changed.
        if (this.data && this.loader.compare(this.data, data)) return this.cached;
        this.data = data;
        // Transform the manifest into matchers.
        const matchers = await this.transform(data);
        // Cache the matchers.
        this.cached = matchers;
        return matchers;
    }
}

//# sourceMappingURL=cached-route-matcher-provider.js.map