/// <reference types="node" />
import type { BaseNextRequest } from '../../../base-http';
import type { NodeNextRequest } from '../../../base-http/node';
import type { WebNextRequest } from '../../../base-http/web';
import type { Writable } from 'node:stream';
import { NextRequest } from '../request';
export declare const ResponseAbortedName = "ResponseAborted";
export declare class ResponseAborted extends Error {
    readonly name = "ResponseAborted";
}
/**
 * Creates an AbortController tied to the closing of a ServerResponse (or other
 * appropriate Writable).
 *
 * If the `close` event is fired before the `finish` event, then we'll send the
 * `abort` signal.
 */
export declare function createAbortController(response: Writable): AbortController;
/**
 * Creates an AbortSignal tied to the closing of a ServerResponse (or other
 * appropriate Writable).
 *
 * This cannot be done with the request (IncomingMessage or Readable) because
 * the `abort` event will not fire if to data has been fully read (because that
 * will "close" the readable stream and nothing fires after that).
 */
export declare function signalFromNodeResponse(response: Writable): AbortSignal;
export declare class NextRequestAdapter {
    static fromBaseNextRequest(request: BaseNextRequest, signal: AbortSignal): NextRequest;
    static fromNodeNextRequest(request: NodeNextRequest, signal: AbortSignal): NextRequest;
    static fromWebNextRequest(request: WebNextRequest): NextRequest;
}
