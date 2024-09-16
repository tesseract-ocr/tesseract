/// <reference types="node" />
/// <reference types="node" />
import type { IncomingMessage } from 'http';
import type { NextParsedUrlQuery } from '../../../../server/request-meta';
import type { Params } from './route-matcher';
import type { RouteHas } from '../../../../lib/load-custom-routes';
import type { BaseNextRequest } from '../../../../server/base-http';
export declare function matchHas(req: BaseNextRequest | IncomingMessage, query: Params, has?: RouteHas[], missing?: RouteHas[]): false | Params;
export declare function compileNonPath(value: string, params: Params): string;
export declare function prepareDestination(args: {
    appendParamsToQuery: boolean;
    destination: string;
    params: Params;
    query: NextParsedUrlQuery;
}): {
    newUrl: string;
    destQuery: import("querystring").ParsedUrlQuery;
    parsedDestination: import("./parse-url").ParsedUrl;
};
