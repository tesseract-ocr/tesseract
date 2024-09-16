/**
 * For a given path and a locale, if the locale is given, it will prefix the
 * locale. The path shouldn't be an API path. If a default locale is given the
 * prefix will be omitted if the locale is already the default locale.
 */
export declare function addLocale(path: string, locale?: string | false, defaultLocale?: string, ignorePrefix?: boolean): string;
