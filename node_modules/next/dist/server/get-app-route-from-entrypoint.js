"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return getAppRouteFromEntrypoint;
    }
});
const _matchbundle = /*#__PURE__*/ _interop_require_default(require("./match-bundle"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
// matches app/:path*.js
const APP_ROUTE_NAME_REGEX = /^app[/\\](.*)$/;
function getAppRouteFromEntrypoint(entryFile) {
    const pagePath = (0, _matchbundle.default)(APP_ROUTE_NAME_REGEX, entryFile);
    if (typeof pagePath === "string" && !pagePath) {
        return "/";
    }
    if (!pagePath) {
        return null;
    }
    return pagePath;
}

//# sourceMappingURL=get-app-route-from-entrypoint.js.map