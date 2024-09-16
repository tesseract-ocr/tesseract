import type { NextParsedUrlQuery } from './request-meta';
export declare function stripInternalQueries(query: NextParsedUrlQuery): void;
export declare function stripInternalSearchParams<T extends string | URL>(url: T, isEdge: boolean): T;
