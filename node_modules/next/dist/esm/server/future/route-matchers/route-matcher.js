import { isDynamicRoute } from "../../../shared/lib/router/utils";
import { getRouteMatcher } from "../../../shared/lib/router/utils/route-matcher";
import { getRouteRegex } from "../../../shared/lib/router/utils/route-regex";
export class RouteMatcher {
    constructor(definition){
        this.definition = definition;
        if (isDynamicRoute(definition.pathname)) {
            this.dynamic = getRouteMatcher(getRouteRegex(definition.pathname));
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