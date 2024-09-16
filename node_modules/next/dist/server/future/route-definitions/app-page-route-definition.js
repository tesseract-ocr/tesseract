"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isAppPageRouteDefinition", {
    enumerable: true,
    get: function() {
        return isAppPageRouteDefinition;
    }
});
const _routekind = require("../route-kind");
function isAppPageRouteDefinition(definition) {
    return definition.kind === _routekind.RouteKind.APP_PAGE;
}

//# sourceMappingURL=app-page-route-definition.js.map