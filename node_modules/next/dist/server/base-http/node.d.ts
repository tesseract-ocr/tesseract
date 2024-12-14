import type { ServerResponse, IncomingMessage } from 'http';
import type { Writable, Readable } from 'stream';
import { SYMBOL_CLEARED_COOKIES } from '../api-utils';
import type { NextApiRequestCookies } from '../api-utils';
import { NEXT_REQUEST_META } from '../request-meta';
import type { RequestMeta } from '../request-meta';
import { BaseNextRequest, BaseNextResponse, type FetchMetric } from './index';
import type { OutgoingHttpHeaders } from 'node:http';
type Req = IncomingMessage & {
    [NEXT_REQUEST_META]?: RequestMeta;
    cookies?: NextApiRequestCookies;
    fetchMetrics?: FetchMetric[];
};
export declare class NodeNextRequest extends BaseNextRequest<Readable> {
    private _req;
    headers: import("http").IncomingHttpHeaders;
    fetchMetrics: FetchMetric[] | undefined;
    [NEXT_REQUEST_META]: RequestMeta;
    constructor(_req: Req);
    get originalRequest(): Req;
    set originalRequest(value: Req);
    private streaming;
}
export declare class NodeNextResponse extends BaseNextResponse<Writable> {
    private _res;
    private textBody;
    [SYMBOL_CLEARED_COOKIES]?: boolean;
    get originalResponse(): ServerResponse<IncomingMessage> & {
        [SYMBOL_CLEARED_COOKIES]?: boolean;
    };
    constructor(_res: ServerResponse & {
        [SYMBOL_CLEARED_COOKIES]?: boolean;
    });
    get sent(): boolean;
    get statusCode(): number;
    set statusCode(value: number);
    get statusMessage(): string;
    set statusMessage(value: string);
    setHeader(name: string, value: string | string[]): this;
    removeHeader(name: string): this;
    getHeaderValues(name: string): string[] | undefined;
    hasHeader(name: string): boolean;
    getHeader(name: string): string | undefined;
    getHeaders(): OutgoingHttpHeaders;
    appendHeader(name: string, value: string): this;
    body(value: string): this;
    send(): void;
    onClose(callback: () => void): void;
}
export {};
