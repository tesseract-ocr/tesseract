"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    fillMetadataSegment: null,
    normalizeMetadataPageToRoute: null,
    normalizeMetadataRoute: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    fillMetadataSegment: function() {
        return fillMetadataSegment;
    },
    normalizeMetadataPageToRoute: function() {
        return normalizeMetadataPageToRoute;
    },
    normalizeMetadataRoute: function() {
        return normalizeMetadataRoute;
    }
});
const _ismetadataroute = require("./is-metadata-route");
const _path = /*#__PURE__*/ _interop_require_default(require("../../shared/lib/isomorphic/path"));
const _serverutils = require("../../server/server-utils");
const _routeregex = require("../../shared/lib/router/utils/route-regex");
const _hash = require("../../shared/lib/hash");
const _apppaths = require("../../shared/lib/router/utils/app-paths");
const _normalizepathsep = require("../../shared/lib/page-path/normalize-path-sep");
const _segment = require("../../shared/lib/segment");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
/*
 * If there's special convention like (...) or @ in the page path,
 * Give it a unique hash suffix to avoid conflicts
 *
 * e.g.
 * /opengraph-image -> /opengraph-image
 * /(post)/opengraph-image.tsx -> /opengraph-image-[0-9a-z]{6}
 *
 * Sitemap is an exception, it should not have a suffix.
 * Each sitemap contains all the urls of sub routes, we don't have the case of duplicates `/(group)/sitemap.[ext]` and `/sitemap.[ext]` since they should be the same.
 * Hence we always normalize the urls for sitemap and do not append hash suffix, and ensure user-land only contains one sitemap per pathname.
 *
 * /sitemap -> /sitemap
 * /(post)/sitemap -> /sitemap
 */ function getMetadataRouteSuffix(page) {
    // Remove the last segment and get the parent pathname
    // e.g. /parent/a/b/c -> /parent/a/b
    // e.g. /parent/opengraph-image -> /parent
    const parentPathname = _path.default.dirname(page);
    // Only apply suffix to metadata routes except for sitemaps
    if (page.endsWith('/sitemap')) {
        return '';
    }
    // Calculate the hash suffix based on the parent path
    let suffix = '';
    // Check if there's any special characters in the parent pathname.
    const segments = parentPathname.split('/');
    if (segments.some((seg)=>(0, _segment.isGroupSegment)(seg) || (0, _segment.isParallelRouteSegment)(seg))) {
        // Hash the parent path to get a unique suffix
        suffix = (0, _hash.djb2Hash)(parentPathname).toString(36).slice(0, 6);
    }
    return suffix;
}
function fillMetadataSegment(segment, params, lastSegment) {
    const pathname = (0, _apppaths.normalizeAppPath)(segment);
    const routeRegex = (0, _routeregex.getNamedRouteRegex)(pathname, false);
    const route = (0, _serverutils.interpolateDynamicPath)(pathname, params, routeRegex);
    const { name, ext } = _path.default.parse(lastSegment);
    const pagePath = _path.default.posix.join(segment, name);
    const suffix = getMetadataRouteSuffix(pagePath);
    const routeSuffix = suffix ? `-${suffix}` : '';
    return (0, _normalizepathsep.normalizePathSep)(_path.default.join(route, `${name}${routeSuffix}${ext}`));
}
function normalizeMetadataRoute(page) {
    if (!(0, _ismetadataroute.isMetadataRoute)(page)) {
        return page;
    }
    let route = page;
    let suffix = '';
    if (page === '/robots') {
        route += '.txt';
    } else if (page === '/manifest') {
        route += '.webmanifest';
    } else {
        suffix = getMetadataRouteSuffix(page);
    }
    // Support both /<metadata-route.ext> and custom routes /<metadata-route>/route.ts.
    // If it's a metadata file route, we need to append /[id]/route to the page.
    if (!route.endsWith('/route')) {
        const { dir, name: baseName, ext } = _path.default.parse(route);
        route = _path.default.posix.join(dir, `${baseName}${suffix ? `-${suffix}` : ''}${ext}`, 'route');
    }
    return route;
}
function normalizeMetadataPageToRoute(page, isDynamic) {
    const isRoute = page.endsWith('/route');
    const routePagePath = isRoute ? page.slice(0, -'/route'.length) : page;
    const metadataRouteExtension = routePagePath.endsWith('/sitemap') ? '.xml' : '';
    const mapped = isDynamic ? `${routePagePath}/[__metadata_id__]` : `${routePagePath}${metadataRouteExtension}`;
    return mapped + (isRoute ? '/route' : '');
}

//# sourceMappingURL=get-metadata-route.js.map