import type { RouteKind } from '../route-kind';
import type { RouteMatch } from '../route-matches/route-match';
import type { RouteDefinition } from '../route-definitions/route-definition';
import type { RouteMatcherProvider } from '../route-matcher-providers/route-matcher-provider';
import type { RouteMatcher } from '../route-matchers/route-matcher';
import type { MatchOptions, RouteMatcherManager } from './route-matcher-manager';
interface RouteMatchers {
    static: ReadonlyArray<RouteMatcher>;
    dynamic: ReadonlyArray<RouteMatcher>;
    duplicates: Record<string, ReadonlyArray<RouteMatcher>>;
}
export declare class DefaultRouteMatcherManager implements RouteMatcherManager {
    private readonly providers;
    protected readonly matchers: RouteMatchers;
    private lastCompilationID;
    /**
     * When this value changes, it indicates that a change has been introduced
     * that requires recompilation.
     */
    private get compilationID();
    private waitTillReadyPromise?;
    waitTillReady(): Promise<void>;
    private previousMatchers;
    reload(): Promise<void>;
    push(provider: RouteMatcherProvider): void;
    test(pathname: string, options: MatchOptions): Promise<boolean>;
    match(pathname: string, options: MatchOptions): Promise<RouteMatch<RouteDefinition<RouteKind>> | null>;
    /**
     * This is a point for other managers to override to inject other checking
     * behavior like duplicate route checking on a per-request basis.
     *
     * @param pathname the pathname to validate against
     * @param matcher the matcher to validate/test with
     * @returns the match if found
     */
    protected validate(pathname: string, matcher: RouteMatcher, options: MatchOptions): RouteMatch | null;
    matchAll(pathname: string, options: MatchOptions): AsyncGenerator<RouteMatch<RouteDefinition<RouteKind>>, null, undefined>;
}
export {};
