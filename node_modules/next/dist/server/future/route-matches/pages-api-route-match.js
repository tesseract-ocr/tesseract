"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isPagesAPIRouteMatch", {
    enumerable: true,
    get: function() {
        return isPagesAPIRouteMatch;
    }
});
const _routekind = require("../route-kind");
function isPagesAPIRouteMatch(match) {
    return match.definition.kind === _routekind.RouteKind.PAGES_API;
}

//# sourceMappingURL=pages-api-route-match.js.map