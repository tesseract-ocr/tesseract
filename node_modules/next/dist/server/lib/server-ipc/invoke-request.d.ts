import type { IncomingMessage } from 'http';
import type { Readable } from 'stream';
export declare const invokeRequest: (targetUrl: string, requestInit: {
    headers: IncomingMessage['headers'];
    method: IncomingMessage['method'];
    signal?: AbortSignal;
}, readableBody?: Readable | ReadableStream) => Promise<Response>;
