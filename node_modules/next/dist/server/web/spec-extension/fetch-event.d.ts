import type { WaitUntil } from '../../after/builtin-request-context';
import type { NextRequest } from './request';
declare const responseSymbol: unique symbol;
declare const passThroughSymbol: unique symbol;
declare const waitUntilSymbol: unique symbol;
declare class FetchEvent {
    readonly [waitUntilSymbol]: {
        kind: 'internal';
        promises: Promise<any>[];
    } | {
        kind: 'external';
        function: WaitUntil;
    };
    [responseSymbol]?: Promise<Response>;
    [passThroughSymbol]: boolean;
    constructor(_request: Request, waitUntil?: WaitUntil);
    respondWith(response: Response | Promise<Response>): void;
    passThroughOnException(): void;
    waitUntil(promise: Promise<any>): void;
}
export declare function getWaitUntilPromiseFromEvent(event: FetchEvent): Promise<void> | undefined;
export declare class NextFetchEvent extends FetchEvent {
    sourcePage: string;
    constructor(params: {
        request: NextRequest;
        page: string;
        context: {
            waitUntil: WaitUntil;
        } | undefined;
    });
    /**
     * @deprecated The `request` is now the first parameter and the API is now async.
     *
     * Read more: https://nextjs.org/docs/messages/middleware-new-signature
     */
    get request(): void;
    /**
     * @deprecated Using `respondWith` is no longer needed.
     *
     * Read more: https://nextjs.org/docs/messages/middleware-new-signature
     */
    respondWith(): void;
}
export {};
