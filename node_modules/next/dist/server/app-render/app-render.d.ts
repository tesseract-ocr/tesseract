import type { ActionResult, DynamicParamTypesShort, FlightRouterState, FlightSegmentPath, RenderOpts, Segment, CacheNodeSeedData, PreloadCallbacks } from './types';
import { type WorkStore } from '../app-render/work-async-storage.external';
import type { RequestStore } from '../app-render/work-unit-async-storage.external';
import type { NextParsedUrlQuery } from '../request-meta';
import type { AppPageModule } from '../route-modules/app-page/module';
import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
import type { BaseNextRequest, BaseNextResponse } from '../base-http';
import RenderResult, { type AppPageRenderResultMetadata } from '../render-result';
import { parseRelativeUrl } from '../../shared/lib/router/utils/parse-relative-url';
import type { ServerComponentsHmrCache } from '../response-cache';
import type { FallbackRouteParams } from '../request/fallback-params';
import './clean-async-snapshot.external';
export type GetDynamicParamFromSegment = (segment: string) => {
    param: string;
    value: string | string[] | null;
    treeSegment: Segment;
    type: DynamicParamTypesShort;
} | null;
export type GenerateFlight = typeof generateDynamicFlightRenderResult;
export type AppRenderContext = {
    workStore: WorkStore;
    url: ReturnType<typeof parseRelativeUrl>;
    componentMod: AppPageModule;
    renderOpts: RenderOpts;
    parsedRequestHeaders: ParsedRequestHeaders;
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    query: NextParsedUrlQuery;
    isPrefetch: boolean;
    isAction: boolean;
    requestTimestamp: number;
    appUsingSizeAdjustment: boolean;
    flightRouterState?: FlightRouterState;
    requestId: string;
    pagePath: string;
    clientReferenceManifest: DeepReadonly<ClientReferenceManifest>;
    assetPrefix: string;
    isNotFoundPath: boolean;
    nonce: string | undefined;
    res: BaseNextResponse;
};
interface ParsedRequestHeaders {
    /**
     * Router state provided from the client-side router. Used to handle rendering
     * from the common layout down. This value will be undefined if the request is
     * not a client-side navigation request, or if the request is a prefetch
     * request.
     */
    readonly flightRouterState: FlightRouterState | undefined;
    readonly isPrefetchRequest: boolean;
    readonly isDevWarmupRequest: boolean;
    readonly isHmrRefresh: boolean;
    readonly isRSCRequest: boolean;
    readonly nonce: string | undefined;
}
export type CreateSegmentPath = (child: FlightSegmentPath) => FlightSegmentPath;
/**
 * Produces a RenderResult containing the Flight data for the given request. See
 * `generateDynamicRSCPayload` for information on the contents of the render result.
 */
declare function generateDynamicFlightRenderResult(req: BaseNextRequest, ctx: AppRenderContext, requestStore: RequestStore, options?: {
    actionResult: ActionResult;
    skipFlight: boolean;
    componentTree?: CacheNodeSeedData;
    preloadCallbacks?: PreloadCallbacks;
    temporaryReferences?: WeakMap<any, string>;
}): Promise<RenderResult>;
export type BinaryStreamOf<T> = ReadableStream<Uint8Array>;
export type AppPageRender = (req: BaseNextRequest, res: BaseNextResponse, pagePath: string, query: NextParsedUrlQuery, fallbackRouteParams: FallbackRouteParams | null, renderOpts: RenderOpts, serverComponentsHmrCache: ServerComponentsHmrCache | undefined, isDevWarmup: boolean) => Promise<RenderResult<AppPageRenderResultMetadata>>;
export declare const renderToHTMLOrFlight: AppPageRender;
export declare function warmFlightResponse(flightStream: ReadableStream<Uint8Array>, clientReferenceManifest: DeepReadonly<ClientReferenceManifest>): Promise<unknown>;
export {};
