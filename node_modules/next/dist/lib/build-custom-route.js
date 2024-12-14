"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "buildCustomRoute", {
    enumerable: true,
    get: function() {
        return buildCustomRoute;
    }
});
const _pathtoregexp = require("next/dist/compiled/path-to-regexp");
const _loadcustomroutes = require("./load-custom-routes");
const _redirectstatus = require("./redirect-status");
function buildCustomRoute(type, route, restrictedRedirectPaths) {
    const compiled = (0, _pathtoregexp.pathToRegexp)(route.source, [], {
        strict: true,
        sensitive: false,
        delimiter: '/'
    });
    let source = compiled.source;
    if (!route.internal) {
        source = (0, _redirectstatus.modifyRouteRegex)(source, type === 'redirect' ? restrictedRedirectPaths : undefined);
    }
    const regex = (0, _loadcustomroutes.normalizeRouteRegex)(source);
    if (type !== 'redirect') {
        return {
            ...route,
            regex
        };
    }
    return {
        ...route,
        statusCode: (0, _redirectstatus.getRedirectStatus)(route),
        permanent: undefined,
        regex
    };
}

//# sourceMappingURL=build-custom-route.js.map