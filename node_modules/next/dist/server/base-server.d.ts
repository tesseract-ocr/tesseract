import type { __ApiPreviewProps } from './api-utils';
import type { LoadComponentsReturnType } from './load-components';
import type { MiddlewareRouteMatch } from '../shared/lib/router/utils/middleware-route-matcher';
import type { Params } from './request/params';
import type { NextConfig, NextConfigComplete } from './config-shared';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from './request-meta';
import type { ParsedUrlQuery } from 'querystring';
import type { RenderOptsPartial as PagesRenderOptsPartial } from './render';
import type { RenderOptsPartial as AppRenderOptsPartial, ServerOnInstrumentationRequestError } from './app-render/types';
import { type ServerComponentsHmrCache, type ResponseCacheBase } from './response-cache';
import type { UrlWithParsedQuery } from 'url';
import type { PagesManifest } from '../build/webpack/plugins/pages-manifest-plugin';
import type { BaseNextRequest, BaseNextResponse } from './base-http';
import type { ManifestRewriteRoute, ManifestRoute, PrerenderManifest } from '../build';
import type { ClientReferenceManifest } from '../build/webpack/plugins/flight-manifest-plugin';
import type { NextFontManifest } from '../build/webpack/plugins/next-font-manifest-plugin';
import type { PagesAPIRouteMatch } from './route-matches/pages-api-route-match';
import type { Server as HTTPServer, IncomingMessage, ServerResponse as HTTPServerResponse } from 'http';
import type { MiddlewareMatcher } from '../build/analysis/get-page-static-info';
import type { InstrumentationModule } from './instrumentation/types';
import { type Revalidate, type ExpireTime } from './lib/revalidate';
import RenderResult from './render-result';
import type { RouteMatcherManager } from './route-matcher-managers/route-matcher-manager';
import { LocaleRouteNormalizer } from './normalizers/locale-route-normalizer';
import { I18NProvider } from './lib/i18n-provider';
import { RSCPathnameNormalizer } from './normalizers/request/rsc';
import { PrefetchRSCPathnameNormalizer } from './normalizers/request/prefetch-rsc';
import { NextDataPathnameNormalizer } from './normalizers/request/next-data';
import type { DeepReadonly } from '../shared/lib/deep-readonly';
import { type WaitUntil } from './after/builtin-request-context';
import { FallbackMode } from '../lib/fallback';
export type FindComponentsResult = {
    components: LoadComponentsReturnType;
    query: NextParsedUrlQuery;
};
export interface MiddlewareRoutingItem {
    page: string;
    match: MiddlewareRouteMatch;
    matchers?: MiddlewareMatcher[];
}
export type RouteHandler<ServerRequest extends BaseNextRequest = BaseNextRequest, ServerResponse extends BaseNextResponse = BaseNextResponse> = (req: ServerRequest, res: ServerResponse, parsedUrl: NextUrlWithParsedQuery) => PromiseLike<boolean> | boolean;
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
}
export type RenderOpts = PagesRenderOptsPartial & AppRenderOptsPartial;
export type LoadedRenderOpts = RenderOpts & LoadComponentsReturnType & RequestLifecycleOpts;
export type RequestLifecycleOpts = {
    waitUntil: ((promise: Promise<any>) => void) | undefined;
    onClose: (callback: () => void) => void;
    onAfterTaskError: ((error: unknown) => void) | undefined;
};
type BaseRenderOpts = RenderOpts & {
    poweredByHeader: boolean;
    generateEtags: boolean;
    previewProps: __ApiPreviewProps;
};
/**
 * The public interface for rendering with the server programmatically. This
 * would typically only allow the base request or response to extend it, but
 * because this can be programmatically accessed, we assume that it could also
 * be the base Node.js request and response types.
 */
export interface BaseRequestHandler<ServerRequest extends BaseNextRequest | IncomingMessage = BaseNextRequest, ServerResponse extends BaseNextResponse | HTTPServerResponse = BaseNextResponse> {
    (req: ServerRequest, res: ServerResponse, parsedUrl?: NextUrlWithParsedQuery | undefined): Promise<void> | void;
}
export type RequestContext<ServerRequest extends BaseNextRequest = BaseNextRequest, ServerResponse extends BaseNextResponse = BaseNextResponse> = {
    req: ServerRequest;
    res: ServerResponse;
    pathname: string;
    query: NextParsedUrlQuery;
    renderOpts: RenderOpts;
};
export declare class NoFallbackError extends Error {
}
export declare class WrappedBuildError extends Error {
    innerError: Error;
    constructor(innerError: Error);
}
type ResponsePayload = {
    type: 'html' | 'json' | 'rsc';
    body: RenderResult;
    revalidate?: Revalidate | undefined;
};
export type NextEnabledDirectories = {
    readonly pages: boolean;
    readonly app: boolean;
};
export default abstract class Server<ServerOptions extends Options = Options, ServerRequest extends BaseNextRequest = BaseNextRequest, ServerResponse extends BaseNextResponse = BaseNextResponse> {
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
    protected instrumentation: InstrumentationModule | undefined;
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
    protected abstract getPrerenderManifest(): DeepReadonly<PrerenderManifest>;
    protected abstract getNextFontManifest(): DeepReadonly<NextFontManifest> | undefined;
    protected abstract attachRequestMeta(req: ServerRequest, parsedUrl: NextUrlWithParsedQuery): void;
    protected abstract hasPage(pathname: string): Promise<boolean>;
    protected abstract sendRenderResult(req: ServerRequest, res: ServerResponse, options: {
        result: RenderResult;
        type: 'html' | 'json' | 'rsc';
        generateEtags: boolean;
        poweredByHeader: boolean;
        revalidate: Revalidate | undefined;
        expireTime: ExpireTime | undefined;
    }): Promise<void>;
    protected abstract runApi(req: ServerRequest, res: ServerResponse, query: ParsedUrlQuery, match: PagesAPIRouteMatch): Promise<boolean>;
    protected abstract renderHTML(req: ServerRequest, res: ServerResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: LoadedRenderOpts): Promise<RenderResult>;
    protected abstract getIncrementalCache(options: {
        requestHeaders: Record<string, undefined | string | string[]>;
        requestProtocol: 'http' | 'https';
    }): Promise<import('./lib/incremental-cache').IncrementalCache>;
    protected abstract getResponseCache(options: {
        dev: boolean;
    }): ResponseCacheBase;
    protected getServerComponentsHmrCache(): ServerComponentsHmrCache | undefined;
    protected abstract loadEnvConfig(params: {
        dev: boolean;
        forceReload?: boolean;
    }): void;
    readonly matchers: RouteMatcherManager;
    protected readonly i18nProvider?: I18NProvider;
    protected readonly localeNormalizer?: LocaleRouteNormalizer;
    protected readonly normalizers: {
        readonly rsc: RSCPathnameNormalizer | undefined;
        readonly prefetchRSC: PrefetchRSCPathnameNormalizer | undefined;
        readonly data: NextDataPathnameNormalizer | undefined;
    };
    private readonly isAppPPREnabled;
    /**
     * This is used to persist cache scopes across
     * prefetch -> full route requests for dynamic IO
     * it's only fully used in dev
     */
    constructor(options: ServerOptions);
    protected reloadMatchers(): Promise<void>;
    private handleRSCRequest;
    private handleNextDataRequest;
    protected handleNextImageRequest: RouteHandler<ServerRequest, ServerResponse>;
    protected handleCatchallRenderRequest: RouteHandler<ServerRequest, ServerResponse>;
    protected handleCatchallMiddlewareRequest: RouteHandler<ServerRequest, ServerResponse>;
    protected getRouteMatchers(): RouteMatcherManager;
    protected instrumentationOnRequestError(...args: Parameters<ServerOnInstrumentationRequestError>): Promise<void>;
    logError(err: Error): void;
    handleRequest(req: ServerRequest, res: ServerResponse, parsedUrl?: NextUrlWithParsedQuery): Promise<void>;
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
    getRequestHandler(): BaseRequestHandler<ServerRequest, ServerResponse>;
    protected abstract handleUpgrade(req: ServerRequest, socket: any, head?: any): Promise<void>;
    setAssetPrefix(prefix?: string): void;
    protected prepared: boolean;
    protected preparedPromise: Promise<void> | null;
    /**
     * Runs async initialization of server.
     * It is idempotent, won't fire underlying initialization more than once.
     */
    prepare(): Promise<void>;
    protected prepareImpl(): Promise<void>;
    protected loadInstrumentationModule(): Promise<any>;
    close(): Promise<void>;
    protected getAppPathRoutes(): Record<string, string[]>;
    protected run(req: ServerRequest, res: ServerResponse, parsedUrl: UrlWithParsedQuery): Promise<void>;
    private runImpl;
    private pipe;
    private pipeImpl;
    private getStaticHTML;
    render(req: ServerRequest, res: ServerResponse, pathname: string, query?: NextParsedUrlQuery, parsedUrl?: NextUrlWithParsedQuery, internalRender?: boolean): Promise<void>;
    protected getWaitUntil(): WaitUntil | undefined;
    protected getInternalWaitUntil(): WaitUntil | undefined;
    private renderImpl;
    protected getStaticPaths({ pathname, }: {
        pathname: string;
        requestHeaders: import('./lib/incremental-cache').IncrementalCache['requestHeaders'];
        page: string;
        isAppPath: boolean;
    }): Promise<{
        staticPaths?: string[];
        fallbackMode?: FallbackMode;
    }>;
    private renderToResponseWithComponents;
    protected pathCouldBeIntercepted(resolvedPathname: string): boolean;
    protected setVaryHeader(req: ServerRequest, res: ServerResponse, isAppPath: boolean, resolvedPathname: string): void;
    private renderToResponseWithComponentsImpl;
    private stripNextDataPath;
    protected getOriginalAppPaths(route: string): string[] | null;
    protected renderPageComponent(ctx: RequestContext<ServerRequest, ServerResponse>, bubbleNoFallback: boolean): Promise<false | ResponsePayload | null>;
    private renderToResponse;
    protected abstract getMiddleware(): MiddlewareRoutingItem | undefined;
    protected abstract getFallbackErrorComponents(url?: string): Promise<LoadComponentsReturnType | null>;
    protected abstract getRoutesManifest(): NormalizedRouteManifest | undefined;
    private renderToResponseImpl;
    renderToHTML(req: ServerRequest, res: ServerResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    private renderToHTMLImpl;
    renderError(err: Error | null, req: ServerRequest, res: ServerResponse, pathname: string, query?: NextParsedUrlQuery, setHeaders?: boolean): Promise<void>;
    private renderErrorImpl;
    private customErrorNo404Warn;
    private renderErrorToResponse;
    protected renderErrorToResponseImpl(ctx: RequestContext<ServerRequest, ServerResponse>, err: Error | null): Promise<ResponsePayload | null>;
    renderErrorToHTML(err: Error | null, req: ServerRequest, res: ServerResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    render404(req: ServerRequest, res: ServerResponse, parsedUrl?: Pick<NextUrlWithParsedQuery, 'pathname' | 'query'>, setHeaders?: boolean): Promise<void>;
}
export {};
