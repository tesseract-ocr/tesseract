/// <reference types="node" />
/// <reference types="node" />
/// <reference types="node" />
/// <reference types="node" />
/// <reference types="node" />
import type { ServerResponse, OutgoingHttpHeaders, OutgoingHttpHeader, IncomingMessage, IncomingHttpHeaders } from 'http';
import type { Socket } from 'net';
import Stream from 'stream';
interface MockedRequestOptions {
    url: string;
    headers: IncomingHttpHeaders;
    method: string;
    readable?: Stream.Readable;
    socket?: Socket | null;
}
export declare class MockedRequest extends Stream.Readable implements IncomingMessage {
    url: string;
    readonly statusCode?: number | undefined;
    readonly statusMessage?: string | undefined;
    readonly headers: IncomingHttpHeaders;
    readonly method: string;
    readonly httpVersion = "1.0";
    readonly httpVersionMajor = 1;
    readonly httpVersionMinor = 0;
    private bodyReadable?;
    socket: Socket;
    constructor({ url, headers, method, socket, readable, }: MockedRequestOptions);
    get headersDistinct(): NodeJS.Dict<string[]>;
    _read(size: number): void;
    /**
     * The `connection` property is just an alias for the `socket` property.
     *
     * @deprecated — since v13.0.0 - Use socket instead.
     */
    get connection(): Socket;
    get aborted(): boolean;
    get complete(): boolean;
    get trailers(): NodeJS.Dict<string>;
    get trailersDistinct(): NodeJS.Dict<string[]>;
    get rawTrailers(): string[];
    get rawHeaders(): string[];
    setTimeout(): this;
}
export interface MockedResponseOptions {
    statusCode?: number;
    socket?: Socket | null;
    headers?: OutgoingHttpHeaders;
    resWriter?: (chunk: Uint8Array | Buffer | string) => boolean;
}
export declare class MockedResponse extends Stream.Writable implements ServerResponse {
    statusCode: number;
    statusMessage: string;
    finished: boolean;
    headersSent: boolean;
    readonly socket: Socket | null;
    private resWriter;
    readonly headPromise: Promise<void>;
    private headPromiseResolve?;
    constructor(res?: MockedResponseOptions);
    appendHeader(name: string, value: string | string[]): this;
    /**
     * The `connection` property is just an alias for the `socket` property.
     *
     * @deprecated — since v13.0.0 - Use socket instead.
     */
    get connection(): Socket | null;
    write(chunk: Uint8Array | Buffer | string): boolean;
    end(): this;
    /**
     * This method is a no-op because the `MockedResponse` instance is not
     * actually connected to a socket. This method is not specified on the
     * interface type for `ServerResponse` but is called by Node.js.
     *
     * @see https://github.com/nodejs/node/pull/7949
     */
    _implicitHeader(): void;
    _write(chunk: Buffer | string, _encoding: string, callback: () => void): void;
    writeHead(statusCode: number, statusMessage?: string | undefined, headers?: OutgoingHttpHeaders | OutgoingHttpHeader[] | undefined): this;
    writeHead(statusCode: number, headers?: OutgoingHttpHeaders | OutgoingHttpHeader[] | undefined): this;
    hasHeader(name: string): boolean;
    getHeader(name: string): string | undefined;
    getHeaders(): OutgoingHttpHeaders;
    getHeaderNames(): string[];
    setHeader(name: string, value: OutgoingHttpHeader): this;
    removeHeader(name: string): void;
    flushHeaders(): void;
    get strictContentLength(): boolean;
    writeEarlyHints(): void;
    get req(): IncomingMessage;
    assignSocket(): void;
    detachSocket(): void;
    writeContinue(): void;
    writeProcessing(): void;
    get upgrading(): boolean;
    get chunkedEncoding(): boolean;
    get shouldKeepAlive(): boolean;
    get useChunkedEncodingByDefault(): boolean;
    get sendDate(): boolean;
    setTimeout(): this;
    addTrailers(): void;
}
interface RequestResponseMockerOptions {
    url: string;
    headers?: IncomingHttpHeaders;
    method?: string;
    bodyReadable?: Stream.Readable;
    resWriter?: (chunk: Uint8Array | Buffer | string) => boolean;
    socket?: Socket | null;
}
export declare function createRequestResponseMocks({ url, headers, method, bodyReadable, resWriter, socket, }: RequestResponseMockerOptions): {
    req: MockedRequest;
    res: MockedResponse;
};
export {};
