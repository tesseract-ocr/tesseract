import type { BaseNextRequest } from './base-http';
import type { ParsedUrlQuery } from 'querystring';
export declare const stringifyQuery: (req: BaseNextRequest, query: ParsedUrlQuery) => string;
