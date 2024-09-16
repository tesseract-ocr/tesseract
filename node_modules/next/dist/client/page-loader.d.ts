import type { ComponentType } from 'react';
import type { RouteLoader } from './route-loader';
import type { MiddlewareMatcher } from '../build/analysis/get-page-static-info';
declare global {
    interface Window {
        __DEV_MIDDLEWARE_MATCHERS?: MiddlewareMatcher[];
        __DEV_PAGES_MANIFEST?: {
            pages: string[];
        };
        __SSG_MANIFEST_CB?: () => void;
        __SSG_MANIFEST?: Set<string>;
    }
}
export type StyleSheetTuple = {
    href: string;
    text: string;
};
export type GoodPageCache = {
    page: ComponentType;
    mod: any;
    styleSheets: StyleSheetTuple[];
};
export default class PageLoader {
    private buildId;
    private assetPrefix;
    private promisedSsgManifest;
    private promisedDevPagesManifest?;
    private promisedMiddlewareMatchers?;
    routeLoader: RouteLoader;
    constructor(buildId: string, assetPrefix: string);
    getPageList(): string[] | Promise<string[]>;
    getMiddleware(): MiddlewareMatcher[] | Promise<MiddlewareMatcher[]> | undefined;
    getDataHref(params: {
        asPath: string;
        href: string;
        locale?: string | false;
        skipInterpolation?: boolean;
    }): string;
    _isSsg(
    /** the route (file-system path) */
    route: string): Promise<boolean>;
    loadPage(route: string): Promise<GoodPageCache>;
    prefetch(route: string): Promise<void>;
}
