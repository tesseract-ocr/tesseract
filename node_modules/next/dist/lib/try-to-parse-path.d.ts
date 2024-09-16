import type { Token } from 'next/dist/compiled/path-to-regexp';
interface ParseResult {
    error?: any;
    parsedPath: string;
    regexStr?: string;
    route: string;
    tokens?: Token[];
}
/**
 * Attempts to parse a given route with `path-to-regexp` and returns an object
 * with the result. Whenever an error happens on parse, it will print an error
 * attempting to find the error position and showing a link to the docs. When
 * `handleUrl` is set to `true` it will also attempt to parse the route
 * and use the resulting pathname to parse with `path-to-regexp`.
 */
export declare function tryToParsePath(route: string, options?: {
    handleUrl?: boolean;
}): ParseResult;
export {};
