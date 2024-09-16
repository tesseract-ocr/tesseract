"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AppPageRouteMatcher", {
    enumerable: true,
    get: function() {
        return AppPageRouteMatcher;
    }
});
const _routematcher = require("./route-matcher");
class AppPageRouteMatcher extends _routematcher.RouteMatcher {
    get identity() {
        return `${this.definition.pathname}?__nextPage=${this.definition.page}`;
    }
}

//# sourceMappingURL=app-page-route-matcher.js.map