export interface PathLocale {
    detectedLocale?: string;
    pathname: string;
}
/**
 * For a pathname that may include a locale from a list of locales, it
 * removes the locale from the pathname returning it alongside with the
 * detected locale.
 *
 * @param pathname A pathname that may include a locale.
 * @param locales A list of locales.
 * @returns The detected locale and pathname without locale
 */
export declare function normalizeLocalePath(pathname: string, locales?: string[]): PathLocale;
