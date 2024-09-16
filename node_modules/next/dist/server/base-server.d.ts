/// <reference types="node" />
/// <reference types="node" />
/// <reference types="node" />
import type { __ApiPreviewProps } from './api-utils';
import type { FontManifest } from './font-utils';
import type { LoadComponentsReturnType } from './load-components';
import type { MiddlewareRouteMatch } from '../shared/lib/router/utils/middleware-route-matcher';
import type { Params } from '../shared/lib/router/utils/route-matcher';
import type { NextConfig, NextConfigComplete } from './config-shared';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from './request-meta';
import type { ParsedUrlQuery } from 'querystring';
import type { RenderOptsPartial as PagesRenderOptsPartial } from './render';
import type { RenderOptsPartial as AppRenderOptsPartial } from './app-render/types';
import type { ResponseCacheBase } from './response-cache';
import type { UrlWithParsedQuery } from 'url';
import type { PagesManifest } from '../build/webpack/plugins/pages-manifest-plugin';
import type { BaseNextRequest, BaseNextResponse } from './base-http';
import type { ManifestRewriteRoute, ManifestRoute, PrerenderManifest } from '../build';
import type { ClientReferenceManifest } from '../build/webpack/plugins/flight-manifest-plugin';
import type { NextFontManifest } from '../build/webpack/plugins/next-font-manifest-plugin';
import type { PagesAPIRouteMatch } from './future/route-matches/pages-api-route-match';
import type { Server as HTTPServer, IncomingMessage } from 'http';
import type { MiddlewareMatcher } from '../build/analysis/get-page-static-info';
import { type Revalidate, type SwrDelta } from './lib/revalidate';
import RenderResult from './render-result';
import type { RouteMatcherManager } from './future/route-matcher-managers/route-matcher-manager';
import { LocaleRouteNormalizer } from './future/normalizers/locale-route-normalizer';
import { I18NProvider } from './future/helpers/i18n-provider';
import { RSCPathnameNormalizer } from './future/normalizers/request/rsc';
import { PostponedPathnameNormalizer } from './future/normalizers/request/postponed';
import { ActionPathnameNormalizer } from './future/normalizers/request/action';
import { PrefetchRSCPathnameNormalizer } from './future/normalizers/request/prefetch-rsc';
import { NextDataPathnameNormalizer } from './future/normalizers/request/next-data';
import type { DeepReadonly } from '../shared/lib/deep-readonly';
export type FindComponentsResult = {
    components: LoadComponentsReturnType;
    query: NextParsedUrlQuery;
};
export interface MiddlewareRoutingItem {
    page: string;
    match: MiddlewareRouteMatch;
    matchers?: MiddlewareMatcher[];
}
export type RouteHandler = (req: BaseNextRequest, res: BaseNextResponse, parsedUrl: NextUrlWithParsedQuery) => PromiseLike<boolean> | boolean;
/**
 * The normalized route manifest is the same as the route manifest, but with
 * the rewrites normalized to the object shape that the router expects.
 */
export type NormalizedRouteManifest = {
    readonly dynamicRoutes: ReadonlyArray<ManifestRoute>;
    readonly rewrites: {
        readonly beforeFiles: ReadonlyArray<ManifestRewriteRoute>;
        readonly afterFiles: ReadonlyArray<ManifestRewriteRoute>;
        readonly fallback: ReadonlyArray<ManifestRewriteRoute>;
    };
};
export interface Options {
    /**
     * Object containing the configuration next.config.js
     */
    conf: NextConfig;
    /**
     * Set to false when the server was created by Next.js
     */
    customServer?: boolean;
    /**
     * Tells if Next.js is running in dev mode
     */
    dev?: boolean;
    /**
     * Enables the experimental testing mode.
     */
    experimentalTestProxy?: boolean;
    /**
     * Whether or not the dev server is running in experimental HTTPS mode
     */
    experimentalHttpsServer?: boolean;
    /**
     * Where the Next project is located
     */
    dir?: string;
    /**
     * Tells if Next.js is at the platform-level
     */
    minimalMode?: boolean;
    /**
     * Hide error messages containing server information
     */
    quiet?: boolean;
    /**
     * The hostname the server is running behind
     */
    hostname?: string;
    /**
     * The port the server is running behind
     */
    port?: number;
    /**
     * The HTTP Server that Next.js is running behind
     */
    httpServer?: HTTPServer;
    isNodeDebugging?: 'brk' | boolean;
}
export type RenderOpts = PagesRenderOptsPartial & AppRenderOptsPartial;
export type LoadedRenderOpts = RenderOpts & LoadComponentsReturnType;
type BaseRenderOpts = RenderOpts & {
    poweredByHeader: boolean;
    generateEtags: boolean;
    previewProps: __ApiPreviewProps;
};
export interface BaseRequestHandler {
    (req: BaseNextRequest, res: BaseNextResponse, parsedUrl?: NextUrlWithParsedQuery | undefined): Promise<void> | void;
}
export type RequestContext = {
    req: BaseNextRequest;
    res: BaseNextResponse;
    pathname: string;
    query: NextParsedUrlQuery;
    renderOpts: RenderOpts;
};
export type FallbackMode = false | undefined | 'blocking' | 'static';
export declare class NoFallbackError extends Error {
}
export declare class WrappedBuildError extends Error {
    innerError: Error;
    constructor(innerError: Error);
}
type ResponsePayload = {
    type: 'html' | 'json' | 'rsc';
    body: RenderResult;
    revalidate?: Revalidate;
};
export type NextEnabledDirectories = {
    readonly pages: boolean;
    readonly app: boolean;
};
export default abstract class Server<ServerOptions extends Options = Options> {
    readonly hostname?: string;
    readonly fetchHostname?: string;
    readonly port?: number;
    protected readonly dir: string;
    protected readonly quiet: boolean;
    protected readonly nextConfig: NextConfigComplete;
    protected readonly distDir: string;
    protected readonly publicDir: string;
    protected readonly hasStaticDir: boolean;
    protected readonly pagesManifest?: PagesManifest;
    protected readonly appPathsManifest?: PagesManifest;
    protected readonly buildId: string;
    protected readonly minimalMode: boolean;
    protected readonly renderOpts: BaseRenderOpts;
    protected readonly serverOptions: Readonly<ServerOptions>;
    protected readonly appPathRoutes?: Record<string, string[]>;
    protected readonly clientReferenceManifest?: DeepReadonly<ClientReferenceManifest>;
    protected interceptionRoutePatterns: RegExp[];
    protected nextFontManifest?: DeepReadonly<NextFontManifest>;
    private readonly responseCache;
    protected abstract getPublicDir(): string;
    protected abstract getHasStaticDir(): boolean;
    protected abstract getPagesManifest(): PagesManifest | undefined;
    protected abstract getAppPathsManifest(): PagesManifest | undefined;
    protected abstract getBuildId(): string;
    protected abstract getinterceptionRoutePatterns(): RegExp[];
    protected readonly enabledDirectories: NextEnabledDirectories;
    protected abstract getEnabledDirectories(dev: boolean): NextEnabledDirectories;
    protected readonly experimentalTestProxy?: boolean;
    protected abstract findPageComponents(params: {
        page: string;
        query: NextParsedUrlQuery;
        params: Params;
        isAppPath: boolean;
        sriEnabled?: boolean;
        appPaths?: ReadonlyArray<string> | null;
        shouldEnsure?: boolean;
        url?: string;
    }): Promise<FindComponentsResult | null>;
    protected abstract getFontManifest(): DeepReadonly<FontManifest> | undefined;
    protected abstract getPrerenderManifest(): DeepReadonly<PrerenderManifest>;
    protected abstract getNextFontManifest(): DeepReadonly<NextFontManifest> | undefined;
    protected abstract attachRequestMeta(req: BaseNextRequest, parsedUrl: NextUrlWithParsedQuery): void;
    protected abstract getFallback(page: string): Promise<string>;
    protected abstract hasPage(pathname: string): Promise<boolean>;
    protected abstract sendRenderResult(req: BaseNextRequest, res: BaseNextResponse, options: {
        result: RenderResult;
        type: 'html' | 'json' | 'rsc';
        generateEtags: boolean;
        poweredByHeader: boolean;
        revalidate?: Revalidate;
        swrDelta?: SwrDelta;
    }): Promise<void>;
    protected abstract runApi(req: BaseNextRequest, res: BaseNextResponse, query: ParsedUrlQuery, match: PagesAPIRouteMatch): Promise<boolean>;
    protected abstract renderHTML(req: BaseNextRequest, res: BaseNextResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: LoadedRenderOpts): Promise<RenderResult>;
    protected abstract getPrefetchRsc(pathname: string): Promise<string | null>;
    protected abstract getIncrementalCache(options: {
        requestHeaders: Record<string, undefined | string | string[]>;
        requestProtocol: 'http' | 'https';
    }): Promise<import('./lib/incremental-cache').IncrementalCache>;
    protected abstract getResponseCache(options: {
        dev: boolean;
    }): ResponseCacheBase;
    protected abstract loadEnvConfig(params: {
        dev: boolean;
        forceReload?: boolean;
    }): void;
    readonly matchers: RouteMatcherManager;
    protected readonly i18nProvider?: I18NProvider;
    protected readonly localeNormalizer?: LocaleRouteNormalizer;
    protected readonly normalizers: {
        readonly action: ActionPathnameNormalizer | undefined;
        readonly postponed: PostponedPathnameNormalizer | undefined;
        readonly rsc: RSCPathnameNormalizer | undefined;
        readonly prefetchRSC: PrefetchRSCPathnameNormalizer | undefined;
        readonly data: NextDataPathnameNormalizer | undefined;
    };
    constructor(options: ServerOptions);
    protected reloadMatchers(): Promise<void>;
    private handleRSCRequest;
    private handleNextDataRequest;
    protected handleNextImageRequest: RouteHandler;
    protected handleCatchallRenderRequest: RouteHandler;
    protected handleCatchallMiddlewareRequest: RouteHandler;
    protected getRouteMatchers(): RouteMatcherManager;
    logError(err: Error): void;
    handleRequest(req: BaseNextRequest, res: BaseNextResponse, parsedUrl?: NextUrlWithParsedQuery): Promise<void>;
    private handleRequestImpl;
    /**
     * Normalizes a pathname without attaching any metadata from any matched
     * normalizer.
     *
     * @param pathname the pathname to normalize
     * @returns the normalized pathname
     */
    private normalize;
    private normalizeAndAttachMetadata;
    getRequestHandler(): BaseRequestHandler;
    protected abstract handleUpgrade(req: BaseNextRequest, socket: any, head?: any): Promise<void>;
    setAssetPrefix(prefix?: string): void;
    protected prepared: boolean;
    protected preparedPromise: Promise<void> | null;
    /**
     * Runs async initialization of server.
     * It is idempotent, won't fire underlying initialization more than once.
     */
    prepare(): Promise<void>;
    protected prepareImpl(): Promise<void>;
    protected close(): Promise<void>;
    protected getAppPathRoutes(): Record<string, string[]>;
    protected run(req: BaseNextRequest, res: BaseNextResponse, parsedUrl: UrlWithParsedQuery): Promise<void>;
    private runImpl;
    private pipe;
    private pipeImpl;
    private getStaticHTML;
    render(req: BaseNextRequest, res: BaseNextResponse, pathname: string, query?: NextParsedUrlQuery, parsedUrl?: NextUrlWithParsedQuery, internalRender?: boolean): Promise<void>;
    private renderImpl;
    protected getStaticPaths({ pathname, }: {
        pathname: string;
        requestHeaders: import('./lib/incremental-cache').IncrementalCache['requestHeaders'];
        page: string;
        isAppPath: boolean;
    }): Promise<{
        staticPaths?: string[];
        fallbackMode?: 'static' | 'blocking' | false;
    }>;
    private renderToResponseWithComponents;
    protected pathCouldBeIntercepted(resolvedPathname: string): boolean;
    protected setVaryHeader(req: BaseNextRequest, res: BaseNextResponse, isAppPath: boolean, resolvedPathname: string): void;
    private renderToResponseWithComponentsImpl;
    private stripNextDataPath;
    protected getOriginalAppPaths(route: string): string[] | null;
    protected renderPageComponent(ctx: RequestContext, bubbleNoFallback: boolean): Promise<false | ResponsePayload | null>;
    private renderToResponse;
    protected abstract getMiddleware(): MiddlewareRoutingItem | undefined;
    protected abstract getFallbackErrorComponents(url?: string): Promise<LoadComponentsReturnType | null>;
    protected abstract getRoutesManifest(): NormalizedRouteManifest | undefined;
    private renderToResponseImpl;
    renderToHTML(req: BaseNextRequest, res: BaseNextResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    private renderToHTMLImpl;
    renderError(err: Error | null, req: BaseNextRequest, res: BaseNextResponse, pathname: string, query?: NextParsedUrlQuery, setHeaders?: boolean): Promise<void>;
    private renderErrorImpl;
    private customErrorNo404Warn;
    private renderErrorToResponse;
    protected renderErrorToResponseImpl(ctx: RequestContext, err: Error | null): Promise<ResponsePayload | null>;
    renderErrorToHTML(err: Error | null, req: BaseNextRequest, res: BaseNextResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    render404(req: BaseNextRequest, res: BaseNextResponse, parsedUrl?: Pick<NextUrlWithParsedQuery, 'pathname' | 'query'>, setHeaders?: boolean): Promise<void>;
}
export declare function isRSCRequestCheck(req: IncomingMessage | BaseNextRequest): boolean;
export {};
