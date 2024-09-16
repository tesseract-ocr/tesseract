"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getParams", {
    enumerable: true,
    get: function() {
        return getParams;
    }
});
const _routematcher = require("../../shared/lib/router/utils/route-matcher");
const _routeregex = require("../../shared/lib/router/utils/route-regex");
// The last page and matcher that this function handled.
let last = null;
function getParams(page, pathname) {
    // Because this is often called on the output of `getStaticPaths` or similar
    // where the `page` here doesn't change, this will "remember" the last page
    // it created the RegExp for. If it matches, it'll just re-use it.
    let matcher;
    if ((last == null ? void 0 : last.page) === page) {
        matcher = last.matcher;
    } else {
        matcher = (0, _routematcher.getRouteMatcher)((0, _routeregex.getRouteRegex)(page));
    }
    const params = matcher(pathname);
    if (!params) {
        throw new Error(`The provided export path '${pathname}' doesn't match the '${page}' page.\nRead more: https://nextjs.org/docs/messages/export-path-mismatch`);
    }
    return params;
}

//# sourceMappingURL=get-params.js.map