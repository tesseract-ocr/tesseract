import type { LocaleAnalysisResult } from '../helpers/i18n-provider';
import type { LocaleRouteDefinition } from '../route-definitions/locale-route-definition';
import type { LocaleRouteMatch } from '../route-matches/locale-route-match';
import { RouteMatcher } from './route-matcher';
export type LocaleMatcherMatchOptions = {
    /**
     * If defined, this indicates to the matcher that the request should be
     * treated as locale-aware. If this is undefined, it means that this
     * application was not configured for additional locales.
     */
    i18n?: LocaleAnalysisResult;
};
export declare class LocaleRouteMatcher<D extends LocaleRouteDefinition = LocaleRouteDefinition> extends RouteMatcher<D> {
    /**
     * Identity returns the identity part of the matcher. This is used to compare
     * a unique matcher to another. This is also used when sorting dynamic routes,
     * so it must contain the pathname part as well.
     */
    get identity(): string;
    /**
     * Match will attempt to match the given pathname against this route while
     * also taking into account the locale information.
     *
     * @param pathname The pathname to match against.
     * @param options The options to use when matching.
     * @returns The match result, or `null` if there was no match.
     */
    match(pathname: string, options?: LocaleMatcherMatchOptions): LocaleRouteMatch<D> | null;
    /**
     * Test will attempt to match the given pathname against this route while
     * also taking into account the locale information.
     *
     * @param pathname The pathname to match against.
     * @param options The options to use when matching.
     * @returns The match result, or `null` if there was no match.
     */
    test(pathname: string, options?: LocaleMatcherMatchOptions): {
        params?: Record<string, string | string[]> | undefined;
    } | null;
}
