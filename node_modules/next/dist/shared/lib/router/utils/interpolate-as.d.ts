/// <reference types="node" />
import type { ParsedUrlQuery } from 'querystring';
export declare function interpolateAs(route: string, asPathname: string, query: ParsedUrlQuery): {
    params: string[];
    result: string;
};
