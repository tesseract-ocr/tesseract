import type { RouteMatch } from './route-match';
import type { PagesAPIRouteDefinition } from '../route-definitions/pages-api-route-definition';
export interface PagesAPIRouteMatch extends RouteMatch<PagesAPIRouteDefinition> {
}
/**
 * Checks if the given match is a Pages API route match.
 * @param match the match to check
 * @returns true if the match is a Pages API route match, false otherwise
 */
export declare function isPagesAPIRouteMatch(match: RouteMatch): match is PagesAPIRouteMatch;
