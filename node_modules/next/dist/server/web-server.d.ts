import type { WebNextRequest, WebNextResponse } from './base-http/web';
import type RenderResult from './render-result';
import type { NextParsedUrlQuery, NextUrlWithParsedQuery } from './request-meta';
import type { Params } from '../shared/lib/router/utils/route-matcher';
import type { LoadComponentsReturnType } from './load-components';
import type { LoadedRenderOpts, MiddlewareRoutingItem, NormalizedRouteManifest, Options, RouteHandler } from './base-server';
import type { Revalidate, SwrDelta } from './lib/revalidate';
import BaseServer from './base-server';
import WebResponseCache from './response-cache/web';
import { IncrementalCache } from './lib/incremental-cache';
import type { PAGE_TYPES } from '../lib/page-types';
import type { Rewrite } from '../lib/load-custom-routes';
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
export default class NextWebServer extends BaseServer<WebServerOptions> {
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
    protected handleCatchallRenderRequest: RouteHandler;
    protected renderHTML(req: WebNextRequest, res: WebNextResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: LoadedRenderOpts): Promise<RenderResult>;
    protected sendRenderResult(_req: WebNextRequest, res: WebNextResponse, options: {
        result: RenderResult;
        type: 'html' | 'json';
        generateEtags: boolean;
        poweredByHeader: boolean;
        revalidate: Revalidate | undefined;
        swrDelta: SwrDelta | undefined;
    }): Promise<void>;
    protected findPageComponents({ page, query, params, url, }: {
        page: string;
        query: NextParsedUrlQuery;
        params: Params | null;
        isAppPath: boolean;
        url?: string;
    }): Promise<{
        query: {
            [x: string]: any;
            __nextNotFoundSrcPage?: string | undefined;
            __nextDefaultLocale?: string | undefined;
            __nextFallback?: "true" | undefined;
            __nextLocale?: string | undefined;
            __nextInferredLocaleFromDefault?: "1" | undefined;
            __nextSsgPath?: string | undefined;
            _nextBubbleNoFallback?: "1" | undefined;
            __nextDataReq?: "1" | undefined;
            __nextCustomErrorRender?: "1" | undefined;
            _rsc?: string | undefined;
            amp?: "1" | undefined;
        };
        components: LoadComponentsReturnType;
    } | null>;
    protected runApi(): Promise<boolean>;
    protected handleApiRequest(): Promise<boolean>;
    protected loadEnvConfig(): void;
    protected getPublicDir(): string;
    protected getHasStaticDir(): boolean;
    protected getFallback(): Promise<string>;
    protected getFontManifest(): undefined;
    protected handleCompression(): void;
    protected handleUpgrade(): Promise<void>;
    protected getFallbackErrorComponents(_url?: string): Promise<LoadComponentsReturnType | null>;
    protected getRoutesManifest(): NormalizedRouteManifest | undefined;
    protected getMiddleware(): MiddlewareRoutingItem | undefined;
    protected getFilesystemPaths(): Set<string>;
    protected getPrefetchRsc(): Promise<string | null>;
    protected getinterceptionRoutePatterns(): RegExp[];
}
export {};
