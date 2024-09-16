import type { RouteMatch } from '../route-matches/route-match';
import type { RouteDefinition } from '../route-definitions/route-definition';
type RouteMatchResult = {
    params?: Record<string, string | string[]>;
};
export declare class RouteMatcher<D extends RouteDefinition = RouteDefinition> {
    readonly definition: D;
    private readonly dynamic?;
    /**
     * When set, this is an array of all the other matchers that are duplicates of
     * this one. This is used by the managers to warn the users about possible
     * duplicate matches on routes.
     */
    duplicated?: Array<RouteMatcher>;
    constructor(definition: D);
    /**
     * Identity returns the identity part of the matcher. This is used to compare
     * a unique matcher to another. This is also used when sorting dynamic routes,
     * so it must contain the pathname part.
     */
    get identity(): string;
    get isDynamic(): boolean;
    match(pathname: string): RouteMatch<D> | null;
    test(pathname: string): RouteMatchResult | null;
}
export {};
