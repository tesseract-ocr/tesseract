"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    isAppPageRouteModule: null,
    isAppRouteRouteModule: null,
    isPagesAPIRouteModule: null,
    isPagesRouteModule: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    isAppPageRouteModule: function() {
        return isAppPageRouteModule;
    },
    isAppRouteRouteModule: function() {
        return isAppRouteRouteModule;
    },
    isPagesAPIRouteModule: function() {
        return isPagesAPIRouteModule;
    },
    isPagesRouteModule: function() {
        return isPagesRouteModule;
    }
});
const _routekind = require("../route-kind");
function isAppRouteRouteModule(routeModule) {
    return routeModule.definition.kind === _routekind.RouteKind.APP_ROUTE;
}
function isAppPageRouteModule(routeModule) {
    return routeModule.definition.kind === _routekind.RouteKind.APP_PAGE;
}
function isPagesRouteModule(routeModule) {
    return routeModule.definition.kind === _routekind.RouteKind.PAGES;
}
function isPagesAPIRouteModule(routeModule) {
    return routeModule.definition.kind === _routekind.RouteKind.PAGES_API;
}

//# sourceMappingURL=checks.js.map