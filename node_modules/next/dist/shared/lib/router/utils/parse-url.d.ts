import type { ParsedUrlQuery } from 'querystring';
export interface ParsedUrl {
    hash: string;
    hostname?: string | null;
    href: string;
    pathname: string;
    port?: string | null;
    protocol?: string | null;
    query: ParsedUrlQuery;
    search: string;
}
export declare function parseUrl(url: string): ParsedUrl;
