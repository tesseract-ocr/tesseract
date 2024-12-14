import path from '../../../shared/lib/isomorphic/path';
import { normalizePagePath } from '../../../shared/lib/page-path/normalize-page-path';
import { isDynamicRoute } from '../../../shared/lib/router/utils/is-dynamic';
import { getNamedRouteRegex } from '../../../shared/lib/router/utils/route-regex';
import { normalizeRouteRegex } from '../../../lib/load-custom-routes';
import { escapeStringRegexp } from '../../../shared/lib/escape-regexp';
export function buildDataRoute(page, buildId) {
    const pagePath = normalizePagePath(page);
    const dataRoute = path.posix.join('/_next/data', buildId, `${pagePath}.json`);
    let dataRouteRegex;
    let namedDataRouteRegex;
    let routeKeys;
    if (isDynamicRoute(page)) {
        const routeRegex = getNamedRouteRegex(dataRoute.replace(/\.json$/, ''), true);
        dataRouteRegex = normalizeRouteRegex(routeRegex.re.source.replace(/\(\?:\\\/\)\?\$$/, `\\.json$`));
        namedDataRouteRegex = routeRegex.namedRegex.replace(/\(\?:\/\)\?\$$/, `\\.json$`);
        routeKeys = routeRegex.routeKeys;
    } else {
        dataRouteRegex = normalizeRouteRegex(new RegExp(`^${path.posix.join('/_next/data', escapeStringRegexp(buildId), `${pagePath}.json`)}$`).source);
    }
    return {
        page,
        routeKeys,
        dataRouteRegex,
        namedDataRouteRegex
    };
}

//# sourceMappingURL=build-data-route.js.map