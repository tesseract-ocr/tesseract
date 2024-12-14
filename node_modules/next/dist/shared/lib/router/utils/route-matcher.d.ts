import type { RouteRegex } from './route-regex';
import type { Params } from '../../../../server/request/params';
export interface RouteMatchFn {
    (pathname: string | null | undefined): false | Params;
}
export declare function getRouteMatcher({ re, groups }: RouteRegex): RouteMatchFn;
