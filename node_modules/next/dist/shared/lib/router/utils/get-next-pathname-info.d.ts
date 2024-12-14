import type { I18NProvider } from '../../../../server/lib/i18n-provider';
export interface NextPathnameInfo {
    /**
     * The base path in case the pathname included it.
     */
    basePath?: string;
    /**
     * The buildId for when the parsed URL is a data URL. Parsing it can be
     * disabled with the `parseData` option.
     */
    buildId?: string;
    /**
     * If there was a locale in the pathname, this will hold its value.
     */
    locale?: string;
    /**
     * The processed pathname without a base path, locale, or data URL elements
     * when parsing it is enabled.
     */
    pathname: string;
    /**
     * A boolean telling if the pathname had a trailingSlash. This can be only
     * true if trailingSlash is enabled.
     */
    trailingSlash?: boolean;
}
interface Options {
    /**
     * When passed to true, this function will also parse Nextjs data URLs.
     */
    parseData?: boolean;
    /**
     * A partial of the Next.js configuration to parse the URL.
     */
    nextConfig?: {
        basePath?: string;
        i18n?: {
            locales?: string[];
        } | null;
        trailingSlash?: boolean;
    };
    /**
     * If provided, this normalizer will be used to detect the locale instead of
     * the default locale detection.
     */
    i18nProvider?: I18NProvider;
}
export declare function getNextPathnameInfo(pathname: string, options: Options): NextPathnameInfo;
export {};
