import type { ComponentType } from 'react';
import type { DomainLocale } from '../../../server/config';
import type { MittEmitter } from '../mitt';
import type { ParsedUrlQuery } from 'querystring';
import type { RouterEvent } from '../../../client/router';
import type { StyleSheetTuple } from '../../../client/page-loader';
import type { UrlObject } from 'url';
import type PageLoader from '../../../client/page-loader';
import type { NextPageContext, NEXT_DATA } from '../utils';
declare global {
    interface Window {
        __NEXT_DATA__: NEXT_DATA;
    }
}
interface RouteProperties {
    shallow: boolean;
}
interface TransitionOptions {
    shallow?: boolean;
    locale?: string | false;
    scroll?: boolean;
    unstable_skipClientCache?: boolean;
}
interface NextHistoryState {
    url: string;
    as: string;
    options: TransitionOptions;
}
export type HistoryState = null | {
    __NA: true;
    __N?: false;
} | {
    __N: false;
    __NA?: false;
} | ({
    __NA?: false;
    __N: true;
    key: string;
} & NextHistoryState);
interface MiddlewareEffectParams<T extends FetchDataOutput> {
    fetchData?: () => Promise<T>;
    locale?: string;
    asPath: string;
    router: Router;
}
export declare function matchesMiddleware<T extends FetchDataOutput>(options: MiddlewareEffectParams<T>): Promise<boolean>;
export type Url = UrlObject | string;
export type BaseRouter = {
    route: string;
    pathname: string;
    query: ParsedUrlQuery;
    asPath: string;
    basePath: string;
    locale?: string | undefined;
    locales?: string[] | undefined;
    defaultLocale?: string | undefined;
    domainLocales?: DomainLocale[] | undefined;
    isLocaleDomain: boolean;
};
export type NextRouter = BaseRouter & Pick<Router, 'push' | 'replace' | 'reload' | 'back' | 'forward' | 'prefetch' | 'beforePopState' | 'events' | 'isFallback' | 'isReady' | 'isPreview'>;
export type PrefetchOptions = {
    priority?: boolean;
    locale?: string | false;
    unstable_skipClientCache?: boolean;
};
export type PrivateRouteInfo = (Omit<CompletePrivateRouteInfo, 'styleSheets'> & {
    initial: true;
}) | CompletePrivateRouteInfo;
export type CompletePrivateRouteInfo = {
    Component: ComponentType;
    styleSheets: StyleSheetTuple[];
    __N_SSG?: boolean;
    __N_SSP?: boolean;
    props?: Record<string, any>;
    err?: Error;
    error?: any;
    route?: string;
    resolvedAs?: string;
    query?: ParsedUrlQuery;
};
export type AppProps = Pick<CompletePrivateRouteInfo, 'Component' | 'err'> & {
    router: Router;
} & Record<string, any>;
export type AppComponent = ComponentType<AppProps>;
type Subscription = (data: PrivateRouteInfo, App: AppComponent, resetScroll: {
    x: number;
    y: number;
} | null) => Promise<void>;
type BeforePopStateCallback = (state: NextHistoryState) => boolean;
type ComponentLoadCancel = (() => void) | null;
type HistoryMethod = 'replaceState' | 'pushState';
interface FetchDataOutput {
    dataHref: string;
    json: Record<string, any> | null;
    response: Response;
    text: string;
    cacheKey: string;
}
interface NextDataCache {
    [asPath: string]: Promise<FetchDataOutput>;
}
export declare function createKey(): string;
export default class Router implements BaseRouter {
    basePath: string;
    /**
     * Map of all components loaded in `Router`
     */
    components: {
        [pathname: string]: PrivateRouteInfo;
    };
    sdc: NextDataCache;
    sbc: NextDataCache;
    sub: Subscription;
    clc: ComponentLoadCancel;
    pageLoader: PageLoader;
    _bps: BeforePopStateCallback | undefined;
    events: MittEmitter<RouterEvent>;
    _wrapApp: (App: AppComponent) => any;
    isSsr: boolean;
    _inFlightRoute?: string | undefined;
    _shallow?: boolean | undefined;
    locales?: string[] | undefined;
    defaultLocale?: string | undefined;
    domainLocales?: DomainLocale[] | undefined;
    isReady: boolean;
    isLocaleDomain: boolean;
    isFirstPopStateEvent: boolean;
    _initialMatchesMiddlewarePromise: Promise<boolean>;
    _bfl_s?: import('../../lib/bloom-filter').BloomFilter;
    _bfl_d?: import('../../lib/bloom-filter').BloomFilter;
    private state;
    private _key;
    static events: MittEmitter<RouterEvent>;
    constructor(pathname: string, query: ParsedUrlQuery, as: string, { initialProps, pageLoader, App, wrapApp, Component, err, subscription, isFallback, locale, locales, defaultLocale, domainLocales, isPreview, }: {
        subscription: Subscription;
        initialProps: any;
        pageLoader: any;
        Component: ComponentType;
        App: AppComponent;
        wrapApp: (WrapAppComponent: AppComponent) => any;
        err?: Error;
        isFallback: boolean;
        locale?: string;
        locales?: string[];
        defaultLocale?: string;
        domainLocales?: DomainLocale[];
        isPreview?: boolean;
    });
    onPopState: (e: PopStateEvent) => void;
    reload(): void;
    /**
     * Go back in history
     */
    back(): void;
    /**
     * Go forward in history
     */
    forward(): void;
    /**
     * Performs a `pushState` with arguments
     * @param url of the route
     * @param as masks `url` for the browser
     * @param options object you can define `shallow` and other options
     */
    push(url: Url, as?: Url, options?: TransitionOptions): Promise<boolean>;
    /**
     * Performs a `replaceState` with arguments
     * @param url of the route
     * @param as masks `url` for the browser
     * @param options object you can define `shallow` and other options
     */
    replace(url: Url, as?: Url, options?: TransitionOptions): Promise<boolean>;
    _bfl(as: string, resolvedAs?: string, locale?: string | false, skipNavigate?: boolean): Promise<unknown>;
    private change;
    changeState(method: HistoryMethod, url: string, as: string, options?: TransitionOptions): void;
    handleRouteInfoError(err: Error & {
        code?: any;
        cancelled?: boolean;
    }, pathname: string, query: ParsedUrlQuery, as: string, routeProps: RouteProperties, loadErrorFail?: boolean): Promise<CompletePrivateRouteInfo>;
    getRouteInfo({ route: requestedRoute, pathname, query, as, resolvedAs, routeProps, locale, hasMiddleware, isPreview, unstable_skipClientCache, isQueryUpdating, isMiddlewareRewrite, isNotFound, }: {
        route: string;
        pathname: string;
        query: ParsedUrlQuery;
        as: string;
        resolvedAs: string;
        hasMiddleware?: boolean;
        routeProps: RouteProperties;
        locale: string | undefined;
        isPreview: boolean;
        unstable_skipClientCache?: boolean;
        isQueryUpdating?: boolean;
        isMiddlewareRewrite?: boolean;
        isNotFound?: boolean;
    }): Promise<{
        type: "redirect-external";
        destination: string;
    } | {
        type: "redirect-internal";
        newAs: string;
        newUrl: string;
    } | PrivateRouteInfo>;
    private set;
    /**
     * Callback to execute before replacing router state
     * @param cb callback to be executed
     */
    beforePopState(cb: BeforePopStateCallback): void;
    onlyAHashChange(as: string): boolean;
    scrollToHash(as: string): void;
    urlIsNew(asPath: string): boolean;
    /**
     * Prefetch page code, you may wait for the data during page rendering.
     * This feature only works in production!
     * @param url the href of prefetched page
     * @param asPath the as path of the prefetched page
     */
    prefetch(url: string, asPath?: string, options?: PrefetchOptions): Promise<void>;
    fetchComponent(route: string): Promise<import("../../../client/page-loader").GoodPageCache>;
    _getData<T>(fn: () => Promise<T>): Promise<T>;
    getInitialProps(Component: ComponentType, ctx: NextPageContext): Promise<Record<string, any>>;
    get route(): string;
    get pathname(): string;
    get query(): ParsedUrlQuery;
    get asPath(): string;
    get locale(): string | undefined;
    get isFallback(): boolean;
    get isPreview(): boolean;
}
export {};
