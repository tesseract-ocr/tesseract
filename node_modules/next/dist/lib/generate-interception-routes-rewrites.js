"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    generateInterceptionRoutesRewrites: null,
    isInterceptionRouteRewrite: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    generateInterceptionRoutesRewrites: function() {
        return generateInterceptionRoutesRewrites;
    },
    isInterceptionRouteRewrite: function() {
        return isInterceptionRouteRewrite;
    }
});
const _pathtoregexp = require("next/dist/compiled/path-to-regexp");
const _approuterheaders = require("../client/components/app-router-headers");
const _interceptionroutes = require("../server/lib/interception-routes");
// a function that converts normalised paths (e.g. /foo/[bar]/[baz]) to the format expected by pathToRegexp (e.g. /foo/:bar/:baz)
function toPathToRegexpPath(path) {
    return path.replace(/\[\[?([^\]]+)\]\]?/g, (_, capture)=>{
        // path-to-regexp only supports word characters, so we replace any non-word characters with underscores
        const paramName = capture.replace(/\W+/g, '_');
        // handle catch-all segments (e.g. /foo/bar/[...baz] or /foo/bar/[[...baz]])
        if (capture.startsWith('...')) {
            return `:${capture.slice(3)}*`;
        }
        return ':' + paramName;
    });
}
function generateInterceptionRoutesRewrites(appPaths, basePath = '') {
    const rewrites = [];
    for (const appPath of appPaths){
        if ((0, _interceptionroutes.isInterceptionRouteAppPath)(appPath)) {
            const { interceptingRoute, interceptedRoute } = (0, _interceptionroutes.extractInterceptionRouteInformation)(appPath);
            const normalizedInterceptingRoute = `${interceptingRoute !== '/' ? toPathToRegexpPath(interceptingRoute) : ''}/(.*)?`;
            const normalizedInterceptedRoute = toPathToRegexpPath(interceptedRoute);
            const normalizedAppPath = toPathToRegexpPath(appPath);
            // pathToRegexp returns a regex that matches the path, but we need to
            // convert it to a string that can be used in a header value
            // to the format that Next/the proxy expects
            let interceptingRouteRegex = (0, _pathtoregexp.pathToRegexp)(normalizedInterceptingRoute).toString().slice(2, -3);
            rewrites.push({
                source: `${basePath}${normalizedInterceptedRoute}`,
                destination: `${basePath}${normalizedAppPath}`,
                has: [
                    {
                        type: 'header',
                        key: _approuterheaders.NEXT_URL,
                        value: interceptingRouteRegex
                    }
                ]
            });
        }
    }
    return rewrites;
}
function isInterceptionRouteRewrite(route) {
    var _route_has_, _route_has;
    // When we generate interception rewrites in the above implementation, we always do so with only a single `has` condition.
    return ((_route_has = route.has) == null ? void 0 : (_route_has_ = _route_has[0]) == null ? void 0 : _route_has_.key) === _approuterheaders.NEXT_URL;
}

//# sourceMappingURL=generate-interception-routes-rewrites.js.map