"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "RouteMatcher", {
    enumerable: true,
    get: function() {
        return RouteMatcher;
    }
});
const _utils = require("../../../shared/lib/router/utils");
const _routematcher = require("../../../shared/lib/router/utils/route-matcher");
const _routeregex = require("../../../shared/lib/router/utils/route-regex");
class RouteMatcher {
    constructor(definition){
        this.definition = definition;
        if ((0, _utils.isDynamicRoute)(definition.pathname)) {
            this.dynamic = (0, _routematcher.getRouteMatcher)((0, _routeregex.getRouteRegex)(definition.pathname));
        }
    }
    /**
   * Identity returns the identity part of the matcher. This is used to compare
   * a unique matcher to another. This is also used when sorting dynamic routes,
   * so it must contain the pathname part.
   */ get identity() {
        return this.definition.pathname;
    }
    get isDynamic() {
        return this.dynamic !== undefined;
    }
    match(pathname) {
        const result = this.test(pathname);
        if (!result) return null;
        return {
            definition: this.definition,
            params: result.params
        };
    }
    test(pathname) {
        if (this.dynamic) {
            const params = this.dynamic(pathname);
            if (!params) return null;
            return {
                params
            };
        }
        if (pathname === this.definition.pathname) {
            return {};
        }
        return null;
    }
}

//# sourceMappingURL=route-matcher.js.map