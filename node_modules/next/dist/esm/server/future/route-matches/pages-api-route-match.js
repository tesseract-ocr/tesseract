import { RouteKind } from "../route-kind";
/**
 * Checks if the given match is a Pages API route match.
 * @param match the match to check
 * @returns true if the match is a Pages API route match, false otherwise
 */ export function isPagesAPIRouteMatch(match) {
    return match.definition.kind === RouteKind.PAGES_API;
}

//# sourceMappingURL=pages-api-route-match.js.map