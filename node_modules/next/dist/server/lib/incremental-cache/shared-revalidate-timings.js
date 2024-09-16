"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "SharedRevalidateTimings", {
    enumerable: true,
    get: function() {
        return SharedRevalidateTimings;
    }
});
class SharedRevalidateTimings {
    static #_ = /**
   * The in-memory cache of revalidate timings for routes. This cache is
   * populated when the cache is updated with new timings.
   */ this.timings = new Map();
    constructor(/**
     * The prerender manifest that contains the initial revalidate timings for
     * routes.
     */ prerenderManifest){
        this.prerenderManifest = prerenderManifest;
    }
    /**
   * Try to get the revalidate timings for a route. This will first try to get
   * the timings from the in-memory cache. If the timings are not present in the
   * in-memory cache, then the timings will be sourced from the prerender
   * manifest.
   *
   * @param route the route to get the revalidate timings for
   * @returns the revalidate timings for the route, or undefined if the timings
   *          are not present in the in-memory cache or the prerender manifest
   */ get(route) {
        var _this_prerenderManifest_routes_route;
        // This is a copy on write cache that is updated when the cache is updated.
        // If the cache is never written to, then the timings will be sourced from
        // the prerender manifest.
        let revalidate = SharedRevalidateTimings.timings.get(route);
        if (typeof revalidate !== "undefined") return revalidate;
        revalidate = (_this_prerenderManifest_routes_route = this.prerenderManifest.routes[route]) == null ? void 0 : _this_prerenderManifest_routes_route.initialRevalidateSeconds;
        if (typeof revalidate !== "undefined") return revalidate;
        return undefined;
    }
    /**
   * Set the revalidate timings for a route.
   *
   * @param route the route to set the revalidate timings for
   * @param revalidate the revalidate timings for the route
   */ set(route, revalidate) {
        SharedRevalidateTimings.timings.set(route, revalidate);
    }
    /**
   * Clear the in-memory cache of revalidate timings for routes.
   */ clear() {
        SharedRevalidateTimings.timings.clear();
    }
}

//# sourceMappingURL=shared-revalidate-timings.js.map