import { pathToRegexp } from 'next/dist/compiled/path-to-regexp';
import { normalizeRouteRegex } from './load-custom-routes';
import { getRedirectStatus, modifyRouteRegex } from './redirect-status';
export function buildCustomRoute(type, route, restrictedRedirectPaths) {
    const compiled = pathToRegexp(route.source, [], {
        strict: true,
        sensitive: false,
        delimiter: '/'
    });
    let source = compiled.source;
    if (!route.internal) {
        source = modifyRouteRegex(source, type === 'redirect' ? restrictedRedirectPaths : undefined);
    }
    const regex = normalizeRouteRegex(source);
    if (type !== 'redirect') {
        return {
            ...route,
            regex
        };
    }
    return {
        ...route,
        statusCode: getRedirectStatus(route),
        permanent: undefined,
        regex
    };
}

//# sourceMappingURL=build-custom-route.js.map