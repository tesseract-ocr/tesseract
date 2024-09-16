import type { DomainLocale, I18NConfig } from '../../config-shared';
import type { NextParsedUrlQuery } from '../../request-meta';
/**
 * The result of matching a locale aware route.
 */
export interface LocaleAnalysisResult {
    /**
     * The pathname without the locale prefix (if any).
     */
    pathname: string;
    /**
     * The detected locale. If no locale was detected, this will be `undefined`.
     */
    detectedLocale?: string;
    /**
     * True if the locale was inferred from the default locale.
     */
    inferredFromDefault: boolean;
}
type LocaleAnalysisOptions = {
    /**
     * When provided, it will be used as the default locale if the locale
     * cannot be inferred from the pathname.
     */
    defaultLocale?: string;
};
/**
 * The I18NProvider is used to match locale aware routes, detect the locale from
 * the pathname and hostname and normalize the pathname by removing the locale
 * prefix.
 */
export declare class I18NProvider {
    readonly config: Readonly<I18NConfig>;
    private readonly lowerCaseLocales;
    private readonly lowerCaseDomains?;
    constructor(config: Readonly<I18NConfig>);
    /**
     * Detects the domain locale from the hostname and the detected locale if
     * provided.
     *
     * @param hostname The hostname to detect the domain locale from, this must be lowercased.
     * @param detectedLocale The detected locale to use if the hostname does not match.
     * @returns The domain locale if found, `undefined` otherwise.
     */
    detectDomainLocale(hostname?: string, detectedLocale?: string): DomainLocale | undefined;
    /**
     * Pulls the pre-computed locale and inference results from the query
     * object.
     *
     * @param pathname the pathname that could contain a locale prefix
     * @param query the query object
     * @returns the locale analysis result
     */
    fromQuery(pathname: string, query: NextParsedUrlQuery): LocaleAnalysisResult;
    /**
     * Analyzes the pathname for a locale and returns the pathname without it.
     *
     * @param pathname The pathname that could contain a locale prefix.
     * @param options The options to use when matching the locale.
     * @returns The matched locale and the pathname without the locale prefix
     *          (if any).
     */
    analyze(pathname: string, options?: LocaleAnalysisOptions): LocaleAnalysisResult;
}
export {};
