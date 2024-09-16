import type { I18NConfig } from '../../config-shared';
import { NextURL } from '../next-url';
import { ResponseCookies } from './cookies';
declare const INTERNALS: unique symbol;
/**
 * This class extends the [Web `Response` API](https://developer.mozilla.org/docs/Web/API/Response) with additional convenience methods.
 *
 * Read more: [Next.js Docs: `NextResponse`](https://nextjs.org/docs/app/api-reference/functions/next-response)
 */
export declare class NextResponse<Body = unknown> extends Response {
    [INTERNALS]: {
        cookies: ResponseCookies;
        url?: NextURL;
        body?: Body;
    };
    constructor(body?: BodyInit | null, init?: ResponseInit);
    get cookies(): ResponseCookies;
    static json<JsonBody>(body: JsonBody, init?: ResponseInit): NextResponse<JsonBody>;
    static redirect(url: string | NextURL | URL, init?: number | ResponseInit): NextResponse<unknown>;
    static rewrite(destination: string | NextURL | URL, init?: MiddlewareResponseInit): NextResponse<unknown>;
    static next(init?: MiddlewareResponseInit): NextResponse<unknown>;
}
interface ResponseInit extends globalThis.ResponseInit {
    nextConfig?: {
        basePath?: string;
        i18n?: I18NConfig;
        trailingSlash?: boolean;
    };
    url?: string;
}
interface ModifiedRequest {
    /**
     * If this is set, the request headers will be overridden with this value.
     */
    headers?: Headers;
}
interface MiddlewareResponseInit extends globalThis.ResponseInit {
    /**
     * These fields will override the request from clients.
     */
    request?: ModifiedRequest;
}
export {};
