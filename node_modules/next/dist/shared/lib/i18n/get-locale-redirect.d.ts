import type { DomainLocale } from '../../../server/config';
import type { I18NConfig } from '../../../server/config-shared';
interface Options {
    defaultLocale: string;
    domainLocale?: DomainLocale;
    headers?: {
        [key: string]: string | string[] | undefined;
    };
    nextConfig: {
        basePath?: string;
        i18n?: I18NConfig | null;
        trailingSlash?: boolean;
    };
    pathLocale?: string;
    urlParsed: {
        hostname?: string | null;
        pathname: string;
    };
}
export declare function getLocaleRedirect({ defaultLocale, domainLocale, pathLocale, headers, nextConfig, urlParsed, }: Options): string | undefined;
export {};
