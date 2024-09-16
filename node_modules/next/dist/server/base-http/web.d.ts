/// <reference types="node" />
import type { IncomingHttpHeaders, OutgoingHttpHeaders } from 'http';
import type { FetchMetrics } from './index';
import { BaseNextRequest, BaseNextResponse } from './index';
import type { NextRequestHint } from '../web/adapter';
export declare class WebNextRequest extends BaseNextRequest<ReadableStream | null> {
    request: Request;
    headers: IncomingHttpHeaders;
    fetchMetrics?: FetchMetrics;
    constructor(request: NextRequestHint);
    parseBody(_limit: string | number): Promise<any>;
}
export declare class WebNextResponse extends BaseNextResponse<WritableStream> {
    transformStream: TransformStream<any, any>;
    private headers;
    private textBody;
    statusCode: number | undefined;
    statusMessage: string | undefined;
    constructor(transformStream?: TransformStream<any, any>);
    setHeader(name: string, value: string | string[]): this;
    removeHeader(name: string): this;
    getHeaderValues(name: string): string[] | undefined;
    getHeader(name: string): string | undefined;
    getHeaders(): OutgoingHttpHeaders;
    hasHeader(name: string): boolean;
    appendHeader(name: string, value: string): this;
    body(value: string): this;
    private readonly sendPromise;
    private _sent;
    send(): void;
    get sent(): boolean;
    toResponse(): Promise<Response>;
}
