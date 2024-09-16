import type { RouteRegex } from './route-regex';
export interface RouteMatchFn {
    (pathname: string | null | undefined): false | Params;
}
export interface Params {
    [param: string]: any;
}
export declare function getRouteMatcher({ re, groups }: RouteRegex): RouteMatchFn;
