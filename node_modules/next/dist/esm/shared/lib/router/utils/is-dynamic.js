import { extractInterceptionRouteInformation, isInterceptionRouteAppPath } from "../../../../server/future/helpers/interception-routes";
// Identify /[param]/ in route string
const TEST_ROUTE = /\/\[[^/]+?\](?=\/|$)/;
export function isDynamicRoute(route) {
    if (isInterceptionRouteAppPath(route)) {
        route = extractInterceptionRouteInformation(route).interceptedRoute;
    }
    return TEST_ROUTE.test(route);
}

//# sourceMappingURL=is-dynamic.js.map