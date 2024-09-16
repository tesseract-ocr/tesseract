"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getMiddlewareRouteMatcher", {
    enumerable: true,
    get: function() {
        return getMiddlewareRouteMatcher;
    }
});
const _preparedestination = require("./prepare-destination");
function getMiddlewareRouteMatcher(matchers) {
    return (pathname, req, query)=>{
        for (const matcher of matchers){
            const routeMatch = new RegExp(matcher.regexp).exec(pathname);
            if (!routeMatch) {
                continue;
            }
            if (matcher.has || matcher.missing) {
                const hasParams = (0, _preparedestination.matchHas)(req, query, matcher.has, matcher.missing);
                if (!hasParams) {
                    continue;
                }
            }
            return true;
        }
        return false;
    };
}

//# sourceMappingURL=middleware-route-matcher.js.map