import { RouteKind } from "../route-kind";
export function isAppRouteRouteModule(routeModule) {
    return routeModule.definition.kind === RouteKind.APP_ROUTE;
}
export function isAppPageRouteModule(routeModule) {
    return routeModule.definition.kind === RouteKind.APP_PAGE;
}
export function isPagesRouteModule(routeModule) {
    return routeModule.definition.kind === RouteKind.PAGES;
}
export function isPagesAPIRouteModule(routeModule) {
    return routeModule.definition.kind === RouteKind.PAGES_API;
}

//# sourceMappingURL=checks.js.map