import './node-environment';
import './require-hook';
import './node-polyfill-crypto';
import type { CacheFs } from '../shared/lib/utils';
import type { MiddlewareManifest } from '../build/webpack/plugins/middleware-plugin';
import type RenderResult from './render-result';
import type { FetchEventResult } from './web/types';
import type { PrerenderManifest } from '../build';
import type { PagesManifest } from '../build/webpack/plugins/pages-manifest-plugin';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from './request-meta';
import type { Params } from './request/params';
import type { RouteMatch } from './route-matches/route-match';
import type { IncomingMessage, ServerResponse } from 'http';
import type { UrlWithParsedQuery } from 'url';
import type { ParsedUrlQuery } from 'querystring';
import type { ParsedUrl } from '../shared/lib/router/utils/parse-url';
import type { Revalidate, ExpireTime } from './lib/revalidate';
import type { WaitUntil } from './after/builtin-request-context';
import { NodeNextRequest, NodeNextResponse } from './base-http/node';
import type { Options, FindComponentsResult, MiddlewareRoutingItem, RequestContext, NormalizedRouteManifest, LoadedRenderOpts, RouteHandler, NextEnabledDirectories, BaseRequestHandler } from './base-server';
import BaseServer from './base-server';
import type { LoadComponentsReturnType } from './load-components';
import ResponseCache, { type IncrementalCacheItem } from './response-cache';
import { IncrementalCache } from './lib/incremental-cache';
import type { PagesAPIRouteMatch } from './route-matches/pages-api-route-match';
import type { NextFontManifest } from '../build/webpack/plugins/next-font-manifest-plugin';
import type { ServerOnInstrumentationRequestError } from './app-render/types';
import { AsyncCallbackSet } from './lib/async-callback-set';
export * from './base-server';
export type NodeRequestHandler = BaseRequestHandler<IncomingMessage | NodeNextRequest, ServerResponse | NodeNextResponse>;
type NodeRouteHandler = RouteHandler<NodeNextRequest, NodeNextResponse>;
export default class NextNodeServer extends BaseServer<Options, NodeNextRequest, NodeNextResponse> {
    protected middlewareManifestPath: string;
    private _serverDistDir;
    private imageResponseCache?;
    private registeredInstrumentation;
    protected renderWorkersPromises?: Promise<void>;
    protected dynamicRoutes?: {
        match: import('../shared/lib/router/utils/route-matcher').RouteMatchFn;
        page: string;
        re: RegExp;
    }[];
    private routerServerHandler?;
    protected cleanupListeners: AsyncCallbackSet;
    protected internalWaitUntil: WaitUntil | undefined;
    constructor(options: Options);
    unstable_preloadEntries(): Promise<void>;
    protected handleUpgrade(): Promise<void>;
    protected loadInstrumentationModule(): Promise<import("./instrumentation/types").InstrumentationModule | undefined>;
    protected prepareImpl(): Promise<void>;
    protected runInstrumentationHookIfAvailable(): Promise<void>;
    protected loadEnvConfig({ dev, forceReload, silent, }: {
        dev: boolean;
        forceReload?: boolean;
        silent?: boolean;
    }): void;
    protected getIncrementalCache({ requestHeaders, requestProtocol, }: {
        requestHeaders: IncrementalCache['requestHeaders'];
        requestProtocol: 'http' | 'https';
    }): Promise<IncrementalCache>;
    protected getResponseCache(): ResponseCache;
    protected getPublicDir(): string;
    protected getHasStaticDir(): boolean;
    protected getPagesManifest(): PagesManifest | undefined;
    protected getAppPathsManifest(): PagesManifest | undefined;
    protected getinterceptionRoutePatterns(): RegExp[];
    protected hasPage(pathname: string): Promise<boolean>;
    protected getBuildId(): string;
    protected getEnabledDirectories(dev: boolean): NextEnabledDirectories;
    protected sendRenderResult(req: NodeNextRequest, res: NodeNextResponse, options: {
        result: RenderResult;
        type: 'html' | 'json' | 'rsc';
        generateEtags: boolean;
        poweredByHeader: boolean;
        revalidate: Revalidate | undefined;
        expireTime: ExpireTime | undefined;
    }): Promise<void>;
    protected runApi(req: NodeNextRequest, res: NodeNextResponse, query: ParsedUrlQuery, match: PagesAPIRouteMatch): Promise<boolean>;
    protected renderHTML(req: NodeNextRequest, res: NodeNextResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: LoadedRenderOpts): Promise<RenderResult>;
    private renderHTMLImpl;
    protected imageOptimizer(req: NodeNextRequest, res: NodeNextResponse, paramsResult: import('./image-optimizer').ImageParamsResult, previousCacheEntry?: IncrementalCacheItem): Promise<{
        buffer: Buffer;
        contentType: string;
        maxAge: number;
        upstreamEtag: string;
        etag: string;
    }>;
    protected getPagePath(pathname: string, locales?: string[]): string;
    protected renderPageComponent(ctx: RequestContext<NodeNextRequest, NodeNextResponse>, bubbleNoFallback: boolean): Promise<false | {
        type: "html" | "json" | "rsc";
        body: RenderResult;
        revalidate?: Revalidate | undefined;
    } | null>;
    protected findPageComponents({ page, query, params, isAppPath, url, }: {
        page: string;
        query: NextParsedUrlQuery;
        params: Params;
        isAppPath: boolean;
        sriEnabled?: boolean;
        appPaths?: ReadonlyArray<string> | null;
        shouldEnsure: boolean;
        url?: string;
    }): Promise<FindComponentsResult | null>;
    private findPageComponentsImpl;
    protected getNextFontManifest(): NextFontManifest | undefined;
    protected handleNextImageRequest: NodeRouteHandler;
    protected handleCatchallRenderRequest: NodeRouteHandler;
    protected logErrorWithOriginalStack(_err?: unknown, _type?: 'unhandledRejection' | 'uncaughtException' | 'warning' | 'app-dir'): void;
    protected ensurePage(_opts: {
        page: string;
        clientOnly: boolean;
        appPaths?: ReadonlyArray<string> | null;
        match?: RouteMatch;
        url?: string;
    }): Promise<void>;
    /**
     * Resolves `API` request, in development builds on demand
     * @param req http request
     * @param res http response
     * @param pathname path of request
     */
    protected handleApiRequest(req: NodeNextRequest, res: NodeNextResponse, query: ParsedUrlQuery, match: PagesAPIRouteMatch): Promise<boolean>;
    protected getCacheFilesystem(): CacheFs;
    protected normalizeReq(req: NodeNextRequest | IncomingMessage): NodeNextRequest;
    protected normalizeRes(res: NodeNextResponse | ServerResponse): NodeNextResponse;
    getRequestHandler(): NodeRequestHandler;
    private makeRequestHandler;
    revalidate({ urlPath, revalidateHeaders, opts, }: {
        urlPath: string;
        revalidateHeaders: {
            [key: string]: string | string[];
        };
        opts: {
            unstable_onlyGenerated?: boolean;
        };
    }): Promise<void>;
    render(req: NodeNextRequest | IncomingMessage, res: NodeNextResponse | ServerResponse, pathname: string, query?: NextParsedUrlQuery, parsedUrl?: NextUrlWithParsedQuery, internal?: boolean): Promise<void>;
    renderToHTML(req: NodeNextRequest | IncomingMessage, res: NodeNextResponse | ServerResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    protected renderErrorToResponseImpl(ctx: RequestContext<NodeNextRequest, NodeNextResponse>, err: Error | null): Promise<{
        type: "html" | "json" | "rsc";
        body: RenderResult;
        revalidate?: Revalidate | undefined;
    } | null>;
    renderError(err: Error | null, req: NodeNextRequest | IncomingMessage, res: NodeNextResponse | ServerResponse, pathname: string, query?: NextParsedUrlQuery, setHeaders?: boolean): Promise<void>;
    renderErrorToHTML(err: Error | null, req: NodeNextRequest | IncomingMessage, res: NodeNextResponse | ServerResponse, pathname: string, query?: ParsedUrlQuery): Promise<string | null>;
    render404(req: NodeNextRequest | IncomingMessage, res: NodeNextResponse | ServerResponse, parsedUrl?: NextUrlWithParsedQuery, setHeaders?: boolean): Promise<void>;
    protected getMiddlewareManifest(): MiddlewareManifest | null;
    /** Returns the middleware routing item if there is one. */
    protected getMiddleware(): MiddlewareRoutingItem | undefined;
    protected getEdgeFunctionsPages(): string[];
    /**
     * Get information for the edge function located in the provided page
     * folder. If the edge function info can't be found it will throw
     * an error.
     */
    protected getEdgeFunctionInfo(params: {
        page: string;
        /** Whether we should look for a middleware or not */
        middleware: boolean;
    }): {
        name: string;
        paths: string[];
        wasm: {
            filePath: string;
            name: string;
        }[];
        env: {
            [key: string]: string;
        };
        assets?: {
            filePath: string;
            name: string;
        }[];
    } | null;
    /**
     * Checks if a middleware exists. This method is useful for the development
     * server where we need to check the filesystem. Here we just check the
     * middleware manifest.
     */
    protected hasMiddleware(pathname: string): Promise<boolean>;
    /**
     * A placeholder for a function to be defined in the development server.
     * It will make sure that the root middleware or an edge function has been compiled
     * so that we can run it.
     */
    protected ensureMiddleware(_url?: string): Promise<void>;
    protected ensureEdgeFunction(_params: {
        page: string;
        appPaths: string[] | null;
        url?: string;
    }): Promise<void>;
    /**
     * This method gets all middleware matchers and execute them when the request
     * matches. It will make sure that each middleware exists and is compiled and
     * ready to be invoked. The development server will decorate it to add warns
     * and errors with rich traces.
     */
    protected runMiddleware(params: {
        request: NodeNextRequest;
        response: NodeNextResponse;
        parsedUrl: ParsedUrl;
        parsed: UrlWithParsedQuery;
        onWarning?: (warning: Error) => void;
    }): Promise<FetchEventResult | {
        finished: boolean;
    }>;
    protected handleCatchallMiddlewareRequest: NodeRouteHandler;
    private _cachedPreviewManifest;
    protected getPrerenderManifest(): PrerenderManifest;
    protected getRoutesManifest(): NormalizedRouteManifest | undefined;
    protected attachRequestMeta(req: NodeNextRequest, parsedUrl: NextUrlWithParsedQuery, isUpgradeReq?: boolean): void;
    protected runEdgeFunction(params: {
        req: NodeNextRequest;
        res: NodeNextResponse;
        query: ParsedUrlQuery;
        params: Params | undefined;
        page: string;
        appPaths: string[] | null;
        match?: RouteMatch;
        onError?: (err: unknown) => void;
        onWarning?: (warning: Error) => void;
    }): Promise<FetchEventResult | null>;
    protected get serverDistDir(): string;
    protected getFallbackErrorComponents(_url?: string): Promise<LoadComponentsReturnType | null>;
    protected instrumentationOnRequestError(...args: Parameters<ServerOnInstrumentationRequestError>): Promise<void>;
    protected onServerClose(listener: () => Promise<void>): void;
    close(): Promise<void>;
    protected getInternalWaitUntil(): WaitUntil;
    private createInternalWaitUntil;
}
