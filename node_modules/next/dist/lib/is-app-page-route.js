"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isAppPageRoute", {
    enumerable: true,
    get: function() {
        return isAppPageRoute;
    }
});
function isAppPageRoute(route) {
    return route.endsWith('/page');
}

//# sourceMappingURL=is-app-page-route.js.map