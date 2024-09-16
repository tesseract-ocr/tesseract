import type { NextConfig } from '../server/config';
export type RouteHas = {
    type: 'header' | 'query' | 'cookie';
    key: string;
    value?: string;
} | {
    type: 'host';
    key?: undefined;
    value: string;
};
export type Rewrite = {
    source: string;
    destination: string;
    basePath?: false;
    locale?: false;
    has?: RouteHas[];
    missing?: RouteHas[];
};
export type Header = {
    source: string;
    basePath?: false;
    locale?: false;
    headers: Array<{
        key: string;
        value: string;
    }>;
    has?: RouteHas[];
    missing?: RouteHas[];
};
export type Redirect = {
    source: string;
    destination: string;
    basePath?: false;
    locale?: false;
    has?: RouteHas[];
    missing?: RouteHas[];
} & ({
    statusCode?: never;
    permanent: boolean;
} | {
    statusCode: number;
    permanent?: never;
});
export type Middleware = {
    source: string;
    locale?: false;
    has?: RouteHas[];
    missing?: RouteHas[];
};
export declare function normalizeRouteRegex(regex: string): string;
export type RouteType = 'rewrite' | 'redirect' | 'header';
export declare function checkCustomRoutes(routes: Redirect[] | Header[] | Rewrite[] | Middleware[], type: RouteType | 'middleware'): void;
export interface CustomRoutes {
    headers: Header[];
    rewrites: {
        fallback: Rewrite[];
        afterFiles: Rewrite[];
        beforeFiles: Rewrite[];
    };
    redirects: Redirect[];
}
export default function loadCustomRoutes(config: NextConfig): Promise<CustomRoutes>;
