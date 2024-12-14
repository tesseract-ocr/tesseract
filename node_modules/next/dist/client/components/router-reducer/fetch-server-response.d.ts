import type { FlightRouterState } from '../../../server/app-render/types';
import { NEXT_ROUTER_PREFETCH_HEADER, NEXT_ROUTER_SEGMENT_PREFETCH_HEADER, NEXT_ROUTER_STATE_TREE_HEADER, NEXT_URL, RSC_HEADER, NEXT_HMR_REFRESH_HEADER } from '../app-router-headers';
import { PrefetchKind } from './router-reducer-types';
import { type NormalizedFlightData } from '../../flight-data-helpers';
export interface FetchServerResponseOptions {
    readonly flightRouterState: FlightRouterState;
    readonly nextUrl: string | null;
    readonly prefetchKind?: PrefetchKind;
    readonly isHmrRefresh?: boolean;
}
export type FetchServerResponseResult = {
    flightData: NormalizedFlightData[] | string;
    canonicalUrl: URL | undefined;
    couldBeIntercepted: boolean;
    prerendered: boolean;
    postponed: boolean;
    staleTime: number;
};
export type RequestHeaders = {
    [RSC_HEADER]?: '1';
    [NEXT_ROUTER_STATE_TREE_HEADER]?: string;
    [NEXT_URL]?: string;
    [NEXT_ROUTER_PREFETCH_HEADER]?: '1';
    [NEXT_ROUTER_SEGMENT_PREFETCH_HEADER]?: string;
    'x-deployment-id'?: string;
    [NEXT_HMR_REFRESH_HEADER]?: '1';
    'Next-Test-Fetch-Priority'?: RequestInit['priority'];
};
export declare function urlToUrlWithoutFlightMarker(url: string): URL;
/**
 * Fetch the flight data for the provided url. Takes in the current router state
 * to decide what to render server-side.
 */
export declare function fetchServerResponse(url: URL, options: FetchServerResponseOptions): Promise<FetchServerResponseResult>;
export declare function createFetch(url: URL, headers: RequestHeaders, fetchPriority: 'auto' | 'high' | 'low' | null): Promise<Response>;
export declare function createFromNextReadableStream(flightStream: ReadableStream<Uint8Array>): Promise<unknown>;
