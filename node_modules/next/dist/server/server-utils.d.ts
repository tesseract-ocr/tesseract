/// <reference types="node" />
import type { Rewrite } from '../lib/load-custom-routes';
import type { RouteMatchFn } from '../shared/lib/router/utils/route-matcher';
import type { NextConfig } from './config';
import type { BaseNextRequest } from './base-http';
import type { ParsedUrlQuery } from 'querystring';
import type { UrlWithParsedQuery } from 'url';
import { getNamedRouteRegex } from '../shared/lib/router/utils/route-regex';
export declare function normalizeVercelUrl(req: BaseNextRequest, trustQuery: boolean, paramKeys?: string[], pageIsDynamic?: boolean, defaultRouteRegex?: ReturnType<typeof getNamedRouteRegex> | undefined): void;
export declare function interpolateDynamicPath(pathname: string, params: ParsedUrlQuery, defaultRouteRegex?: ReturnType<typeof getNamedRouteRegex> | undefined): string;
export declare function normalizeDynamicRouteParams(params: ParsedUrlQuery, ignoreOptional?: boolean, defaultRouteRegex?: ReturnType<typeof getNamedRouteRegex> | undefined, defaultRouteMatches?: ParsedUrlQuery | undefined): {
    params: ParsedUrlQuery;
    hasValidParams: boolean;
};
export declare function getUtils({ page, i18n, basePath, rewrites, pageIsDynamic, trailingSlash, caseSensitive, }: {
    page: string;
    i18n?: NextConfig['i18n'];
    basePath: string;
    rewrites: {
        fallback?: ReadonlyArray<Rewrite>;
        afterFiles?: ReadonlyArray<Rewrite>;
        beforeFiles?: ReadonlyArray<Rewrite>;
    };
    pageIsDynamic: boolean;
    trailingSlash?: boolean;
    caseSensitive: boolean;
}): {
    handleRewrites: (req: BaseNextRequest, parsedUrl: UrlWithParsedQuery) => {};
    defaultRouteRegex: {
        namedRegex: string;
        routeKeys: {
            [named: string]: string;
        };
        groups: {
            [groupName: string]: import("../shared/lib/router/utils/route-regex").Group;
        };
        re: RegExp;
    } | undefined;
    dynamicRouteMatcher: RouteMatchFn | undefined;
    defaultRouteMatches: ParsedUrlQuery | undefined;
    getParamsFromRouteMatches: (req: BaseNextRequest, renderOpts?: any, detectedLocale?: string) => ParsedUrlQuery;
    normalizeDynamicRouteParams: (params: ParsedUrlQuery, ignoreOptional?: boolean) => {
        params: ParsedUrlQuery;
        hasValidParams: boolean;
    };
    normalizeVercelUrl: (req: BaseNextRequest, trustQuery: boolean, paramKeys?: string[]) => void;
    interpolateDynamicPath: (pathname: string, params: Record<string, undefined | string | string[]>) => string;
};
