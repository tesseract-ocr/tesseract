"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return getRouteFromEntrypoint;
    }
});
const _getapproutefromentrypoint = /*#__PURE__*/ _interop_require_default(require("./get-app-route-from-entrypoint"));
const _matchbundle = /*#__PURE__*/ _interop_require_default(require("./match-bundle"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
// matches pages/:page*.js
const SERVER_ROUTE_NAME_REGEX = /^pages[/\\](.*)$/;
// matches static/pages/:page*.js
const BROWSER_ROUTE_NAME_REGEX = /^static[/\\]pages[/\\](.*)$/;
function getRouteFromEntrypoint(entryFile, app) {
    let pagePath = (0, _matchbundle.default)(SERVER_ROUTE_NAME_REGEX, entryFile);
    if (pagePath) {
        return pagePath;
    }
    if (app) {
        pagePath = (0, _getapproutefromentrypoint.default)(entryFile);
        if (pagePath) return pagePath;
    }
    // Potentially the passed item is a browser bundle so we try to match that also
    return (0, _matchbundle.default)(BROWSER_ROUTE_NAME_REGEX, entryFile);
}

//# sourceMappingURL=get-route-from-entrypoint.js.map