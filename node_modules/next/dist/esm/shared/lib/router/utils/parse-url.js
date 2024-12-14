import { searchParamsToUrlQuery } from './querystring';
import { parseRelativeUrl } from './parse-relative-url';
export function parseUrl(url) {
    if (url.startsWith('/')) {
        return parseRelativeUrl(url);
    }
    const parsedURL = new URL(url);
    return {
        hash: parsedURL.hash,
        hostname: parsedURL.hostname,
        href: parsedURL.href,
        pathname: parsedURL.pathname,
        port: parsedURL.port,
        protocol: parsedURL.protocol,
        query: searchParamsToUrlQuery(parsedURL.searchParams),
        search: parsedURL.search
    };
}

//# sourceMappingURL=parse-url.js.map