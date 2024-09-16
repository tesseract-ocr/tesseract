/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type { ActionResult, DynamicParamTypesShort, FlightRouterState, FlightSegmentPath, RenderOpts, Segment } from './types';
import type { StaticGenerationStore } from '../../client/components/static-generation-async-storage.external';
import type { RequestStore } from '../../client/components/request-async-storage.external';
import type { NextParsedUrlQuery } from '../request-meta';
import type { AppPageModule } from '../future/route-modules/app-page/module';
import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { Revalidate } from '../lib/revalidate';
import RenderResult, { type AppPageRenderResultMetadata } from '../render-result';
import { type ErrorHandler } from './create-error-handler';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
export type GetDynamicParamFromSegment = (segment: string) => {
    param: string;
    value: string | string[] | null;
    treeSegment: Segment;
    type: DynamicParamTypesShort;
} | null;
type AppRenderBaseContext = {
    staticGenerationStore: StaticGenerationStore;
    requestStore: RequestStore;
    componentMod: AppPageModule;
    renderOpts: RenderOpts;
};
export type GenerateFlight = typeof generateFlight;
export type AppRenderContext = AppRenderBaseContext & {
    getDynamicParamFromSegment: GetDynamicParamFromSegment;
    query: NextParsedUrlQuery;
    isPrefetch: boolean;
    requestTimestamp: number;
    appUsingSizeAdjustment: boolean;
    flightRouterState?: FlightRouterState;
    requestId: string;
    defaultRevalidate: Revalidate;
    pagePath: string;
    clientReferenceManifest: DeepReadonly<ClientReferenceManifest>;
    assetPrefix: string;
    flightDataRendererErrorHandler: ErrorHandler;
    serverComponentsErrorHandler: ErrorHandler;
    isNotFoundPath: boolean;
    res: ServerResponse;
};
export type CreateSegmentPath = (child: FlightSegmentPath) => FlightSegmentPath;
declare function generateFlight(ctx: AppRenderContext, options?: {
    actionResult: ActionResult;
    skipFlight: boolean;
    asNotFound?: boolean;
}): Promise<RenderResult>;
export type BinaryStreamOf<T> = ReadableStream<Uint8Array>;
export type AppPageRender = (req: IncomingMessage, res: ServerResponse, pagePath: string, query: NextParsedUrlQuery, renderOpts: RenderOpts) => Promise<RenderResult<AppPageRenderResultMetadata>>;
export declare const renderToHTMLOrFlight: AppPageRender;
export {};
