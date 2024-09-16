/// <reference types="node" />
import type { ParsedUrlQuery } from 'querystring';
/**
 * Converts the query into params.
 *
 * @param query the query to convert to params
 * @returns the params
 */
export declare function parsedUrlQueryToParams(query: ParsedUrlQuery): Record<string, string | string[]>;
