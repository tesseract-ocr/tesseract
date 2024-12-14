import type { WebNextRequest, WebNextResponse } from './base-http/web';
import type RenderResult from './render-result';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from './request-meta';
import type { Params } from './request/params';
import type { LoadComponentsReturnType } from './load-components';
import type { LoadedRenderOpts, MiddlewareRoutingItem, NormalizedRouteManifest, Options, RouteHandler } from './base-server';
import type { Revalidate, ExpireTime } from './lib/revalidate';
import BaseServer from './base-server';
import WebResponseCache from './response-cache/web';
import { IncrementalCache } from './lib/incremental-cache';
import type { PAGE_TYPES } from '../lib/page-types';
import type { Rewrite } from '../lib/load-custom-routes';
import type { ServerOnInstrumentationRequestError } from './app-render/types';
interface WebServerOptions extends Options {
    webServerConfig: {
        page: string;
        pathname: string;
        pagesType: PAGE_TYPES;
        loadComponent: (page: string) => Promise<LoadComponentsReturnType | null>;
        extendRenderOpts: Partial<BaseServer['renderOpts']> & Pick<BaseServer['renderOpts'], 'buildId'> & {
            serverActionsManifest?: any;
        };
        renderToHTML: typeof import('./app-render/app-render').renderToHTMLOrFlight | undefined;
        incrementalCacheHandler?: any;
        interceptionRouteRewrites?: Rewrite[];
    };
}
type WebRouteHandler = RouteHandler<WebNextRequest, WebNextResponse>;
export default class NextWebServer extends BaseServer<WebServerOptions, WebNextRequest, WebNextResponse> {
    constructor(options: WebServerOptions);
    protected getIncrementalCache({ requestHeaders, }: {
        requestHeaders: IncrementalCache['requestHeaders'];
    }): Promise<IncrementalCache>;
    protected getResponseCache(): WebResponseCache;
    protected hasPage(page: string): Promise<boolean>;
    protected getBuildId(): string;
    protected getEnabledDirectories(): {
        app: boolean;
        pages: boolean;
    };
    protected getPagesManifest(): {
        [x: string]: string;
    };
    protected getAppPathsManifest(): {
        [x: string]: string;
    };
    protected attachRequestMeta(req: WebNextRequest, parsedUrl: NextUrlWithParsedQuery): void;
    protected getPrerenderManifest(): {
        version: any;
        routes: {};
        dynamicRoutes: {};
        notFoundRoutes: never[];
        preview: {
            previewModeId: string;
            previewModeSigningKey: string;
            previewModeEncryptionKey: string;
        };
    };
    protected getNextFontManifest(): {
        readonly pages: {
            readonly [x: string]: readonly string[];
        };
        readonly app: {
            readonly [x: string]: readonly string[];
        };
        readonly appUsingSizeAdjust: boolean;
        readonly pagesUsingSizeAdjust: boolean;
    } | undefined;
    protected handleCatchallRenderRequest: WebRouteHandler;
    protected renderHTML(req: WebNextRequest, res: WebNextResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: LoadedRenderOpts): Promise<RenderResult>;
    protected sendRenderResult(_req: WebNextRequest, res: WebNextResponse, options: {
        result: RenderResult;
        type: 'html' | 'json';
        generateEtags: boolean;
        poweredByHeader: boolean;
        revalidate: Revalidate | undefined;
        expireTime: ExpireTime | undefined;
    }): Promise<void>;
    protected findPageComponents({ page, query, params, url: _url, }: {
        page: string;
        query: NextParsedUrlQuery;
        params: Params | null;
        isAppPath: boolean;
        url?: string;
    }): Promise<{
        query: {
            [x: string]: string | string[] | undefined;
            __nextNotFoundSrcPage?: string;
            __nextDefaultLocale?: string;
            __nextFallback?: "true";
            __nextLocale?: string;
            __nextInferredLocaleFromDefault?: "1";
            __nextSsgPath?: string;
            _nextBubbleNoFallback?: "1";
            __nextDataReq?: "1";
            __nextCustomErrorRender?: "1";
            _rsc?: string;
            amp?: "1";
        };
        components: LoadComponentsReturnType;
    } | null>;
    protected runApi(): Promise<boolean>;
    protected handleApiRequest(): Promise<boolean>;
    protected loadEnvConfig(): void;
    protected getPublicDir(): string;
    protected getHasStaticDir(): boolean;
    protected getFontManifest(): undefined;
    protected handleCompression(): void;
    protected handleUpgrade(): Promise<void>;
    protected getFallbackErrorComponents(_url?: string): Promise<LoadComponentsReturnType | null>;
    protected getRoutesManifest(): NormalizedRouteManifest | undefined;
    protected getMiddleware(): MiddlewareRoutingItem | undefined;
    protected getFilesystemPaths(): Set<string>;
    protected getinterceptionRoutePatterns(): RegExp[];
    protected loadInstrumentationModule(): Promise<import("./instrumentation/types").InstrumentationModule | undefined>;
    protected instrumentationOnRequestError(...args: Parameters<ServerOnInstrumentationRequestError>): Promise<void>;
}
export {};
