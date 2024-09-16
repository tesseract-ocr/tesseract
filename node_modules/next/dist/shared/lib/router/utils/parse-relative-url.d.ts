/// <reference types="node" />
import type { ParsedUrlQuery } from 'querystring';
export interface ParsedRelativeUrl {
    hash: string;
    href: string;
    pathname: string;
    query: ParsedUrlQuery;
    search: string;
}
/**
 * Parses path-relative urls (e.g. `/hello/world?foo=bar`). If url isn't path-relative
 * (e.g. `./hello`) then at least base must be.
 * Absolute urls are rejected with one exception, in the browser, absolute urls that are on
 * the current origin will be parsed as relative
 */
export declare function parseRelativeUrl(url: string, base?: string): ParsedRelativeUrl;
