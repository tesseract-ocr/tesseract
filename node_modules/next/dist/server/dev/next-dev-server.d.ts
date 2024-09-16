/// <reference types="node" />
/// <reference types="node" />
import type { FindComponentsResult } from '../next-server';
import type { LoadComponentsReturnType } from '../load-components';
import type { Options as ServerOptions } from '../next-server';
import type { Params } from '../../shared/lib/router/utils/route-matcher';
import type { ParsedUrl } from '../../shared/lib/router/utils/parse-url';
import type { ParsedUrlQuery } from 'querystring';
import type { UrlWithParsedQuery } from 'url';
import type { BaseNextRequest, BaseNextResponse } from '../base-http';
import type { MiddlewareRoutingItem } from '../base-server';
import type { RouteDefinition } from '../future/route-definitions/route-definition';
import type { RouteMatcherManager } from '../future/route-matcher-managers/route-matcher-manager';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from '../request-meta';
import type { DevBundlerService } from '../lib/dev-bundler-service';
import type { IncrementalCache } from '../lib/incremental-cache';
import type { NodeNextResponse, NodeNextRequest } from '../base-http/node';
import type { PagesManifest } from '../../build/webpack/plugins/pages-manifest-plugin';
import Server from '../next-server';
import { type Span } from '../../trace';
export interface Options extends ServerOptions {
    /**
     * Tells of Next.js is running from the `next dev` command
     */
    isNextDevCommand?: boolean;
    /**
     * Interface to the development bundler.
     */
    bundlerService: DevBundlerService;
    /**
     * Trace span for server startup.
     */
    startServerSpan: Span;
}
export default class DevServer extends Server {
    /**
     * The promise that resolves when the server is ready. When this is unset
     * the server is ready.
     */
    private ready?;
    protected sortedRoutes?: string[];
    private pagesDir?;
    private appDir?;
    private actualMiddlewareFile?;
    private actualInstrumentationHookFile?;
    private middleware?;
    private originalFetch?;
    private readonly bundlerService;
    private staticPathsCache;
    private startServerSpan;
    protected staticPathsWorker?: {
        [key: string]: any;
    } & {
        loadStaticPaths: typeof import('./static-paths-worker').loadStaticPaths;
    };
    private getStaticPathsWorker;
    constructor(options: Options);
    protected getRouteMatchers(): RouteMatcherManager;
    protected getBuildId(): string;
    protected prepareImpl(): Promise<void>;
    protected close(): Promise<void>;
    protected hasPage(pathname: string): Promise<boolean>;
    runMiddleware(params: {
        request: BaseNextRequest;
        response: BaseNextResponse;
        parsedUrl: ParsedUrl;
        parsed: UrlWithParsedQuery;
        middlewareList: MiddlewareRoutingItem[];
    }): Promise<import("../web/types").FetchEventResult | {
        finished: boolean;
    }>;
    runEdgeFunction(params: {
        req: BaseNextRequest;
        res: BaseNextResponse;
        query: ParsedUrlQuery;
        params: Params | undefined;
        page: string;
        appPaths: string[] | null;
        isAppPath: boolean;
    }): Promise<import("../web/types").FetchEventResult | null>;
    handleRequest(req: BaseNextRequest, res: BaseNextResponse, parsedUrl?: NextUrlWithParsedQuery): Promise<void>;
    run(req: NodeNextRequest, res: NodeNextResponse, parsedUrl: UrlWithParsedQuery): Promise<void>;
    protected logErrorWithOriginalStack(err?: unknown, type?: 'unhandledRejection' | 'uncaughtException' | 'warning' | 'app-dir'): Promise<void>;
    protected getPagesManifest(): PagesManifest | undefined;
    protected getAppPathsManifest(): PagesManifest | undefined;
    protected getinterceptionRoutePatterns(): RegExp[];
    protected getMiddleware(): MiddlewareRoutingItem | undefined;
    protected getNextFontManifest(): undefined;
    protected hasMiddleware(): Promise<boolean>;
    protected ensureMiddleware(url: string): Promise<void>;
    private runInstrumentationHookIfAvailable;
    protected ensureEdgeFunction({ page, appPaths, url, }: {
        page: string;
        appPaths: string[] | null;
        url: string;
    }): Promise<void>;
    generateRoutes(_dev?: boolean): void;
    _filterAmpDevelopmentScript(html: string, event: {
        line: number;
        col: number;
        code: string;
    }): boolean;
    protected getStaticPaths({ pathname, requestHeaders, page, isAppPath, }: {
        pathname: string;
        requestHeaders: IncrementalCache['requestHeaders'];
        page: string;
        isAppPath: boolean;
    }): Promise<{
        staticPaths?: string[];
        fallbackMode?: false | 'static' | 'blocking';
    }>;
    private storeGlobals;
    private restorePatchedGlobals;
    protected ensurePage(opts: {
        page: string;
        clientOnly: boolean;
        appPaths?: ReadonlyArray<string> | null;
        definition: RouteDefinition | undefined;
        url?: string;
    }): Promise<void>;
    protected findPageComponents({ page, query, params, isAppPath, appPaths, shouldEnsure, url, }: {
        page: string;
        query: NextParsedUrlQuery;
        params: Params;
        isAppPath: boolean;
        sriEnabled?: boolean;
        appPaths?: ReadonlyArray<string> | null;
        shouldEnsure: boolean;
        url?: string;
    }): Promise<FindComponentsResult | null>;
    protected getFallbackErrorComponents(url?: string): Promise<LoadComponentsReturnType | null>;
    getCompilationError(page: string): Promise<any>;
}
