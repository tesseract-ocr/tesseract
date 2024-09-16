"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "buildDataRoute", {
    enumerable: true,
    get: function() {
        return buildDataRoute;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/isomorphic/path"));
const _normalizepagepath = require("../../../shared/lib/page-path/normalize-page-path");
const _isdynamic = require("../../../shared/lib/router/utils/is-dynamic");
const _routeregex = require("../../../shared/lib/router/utils/route-regex");
const _loadcustomroutes = require("../../../lib/load-custom-routes");
const _escaperegexp = require("../../../shared/lib/escape-regexp");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function buildDataRoute(page, buildId) {
    const pagePath = (0, _normalizepagepath.normalizePagePath)(page);
    const dataRoute = _path.default.posix.join("/_next/data", buildId, `${pagePath}.json`);
    let dataRouteRegex;
    let namedDataRouteRegex;
    let routeKeys;
    if ((0, _isdynamic.isDynamicRoute)(page)) {
        const routeRegex = (0, _routeregex.getNamedRouteRegex)(dataRoute.replace(/\.json$/, ""), true);
        dataRouteRegex = (0, _loadcustomroutes.normalizeRouteRegex)(routeRegex.re.source.replace(/\(\?:\\\/\)\?\$$/, `\\.json$`));
        namedDataRouteRegex = routeRegex.namedRegex.replace(/\(\?:\/\)\?\$$/, `\\.json$`);
        routeKeys = routeRegex.routeKeys;
    } else {
        dataRouteRegex = (0, _loadcustomroutes.normalizeRouteRegex)(new RegExp(`^${_path.default.posix.join("/_next/data", (0, _escaperegexp.escapeStringRegexp)(buildId), `${pagePath}.json`)}$`).source);
    }
    return {
        page,
        routeKeys,
        dataRouteRegex,
        namedDataRouteRegex
    };
}

//# sourceMappingURL=build-data-route.js.map