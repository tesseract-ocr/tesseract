import type { RequestData, FetchEventResult } from './types';
import type { RequestInit } from './spec-extension/request';
import { NextFetchEvent } from './spec-extension/fetch-event';
import { NextRequest } from './spec-extension/request';
export declare class NextRequestHint extends NextRequest {
    sourcePage: string;
    fetchMetrics?: FetchEventResult['fetchMetrics'];
    constructor(params: {
        init: RequestInit;
        input: Request | string;
        page: string;
    });
    get request(): void;
    respondWith(): void;
    waitUntil(): void;
}
export type AdapterOptions = {
    handler: (req: NextRequestHint, event: NextFetchEvent) => Promise<Response>;
    page: string;
    request: RequestData;
    IncrementalCache?: typeof import('../lib/incremental-cache').IncrementalCache;
};
export declare function adapter(params: AdapterOptions): Promise<FetchEventResult>;
