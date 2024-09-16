import { RouteKind } from "../route-kind";
/**
 * Returns true if the given definition is an App Page route definition.
 */ export function isAppPageRouteDefinition(definition) {
    return definition.kind === RouteKind.APP_PAGE;
}

//# sourceMappingURL=app-page-route-definition.js.map