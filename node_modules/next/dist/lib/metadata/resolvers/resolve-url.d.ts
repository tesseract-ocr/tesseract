import type { MetadataContext } from '../types/resolvers';
declare function isStringOrURL(icon: any): icon is string | URL;
export declare function getSocialImageFallbackMetadataBase(metadataBase: URL | null): {
    fallbackMetadataBase: URL;
    isMetadataBaseMissing: boolean;
};
declare function resolveUrl(url: null | undefined, metadataBase: URL | null): null;
declare function resolveUrl(url: string | URL, metadataBase: URL | null): URL;
declare function resolveUrl(url: string | URL | null | undefined, metadataBase: URL | null): URL | null;
declare function resolveRelativeUrl(url: string | URL, pathname: string): string | URL;
declare function resolveAbsoluteUrlWithPathname(url: string | URL, metadataBase: URL | null, { trailingSlash, pathname }: MetadataContext): string;
export { isStringOrURL, resolveUrl, resolveRelativeUrl, resolveAbsoluteUrlWithPathname, };
