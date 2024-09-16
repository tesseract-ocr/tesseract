import { type ReadonlyRequestCookies } from '../../server/web/spec-extension/adapters/request-cookies';
import { DraftMode } from './draft-mode';
/**
 * This function allows you to read the HTTP incoming request headers in
 * [Server Components](https://nextjs.org/docs/app/building-your-application/rendering/server-components),
 * [Server Actions](https://nextjs.org/docs/app/building-your-application/data-fetching/server-actions-and-mutations),
 * [Route Handlers](https://nextjs.org/docs/app/building-your-application/routing/route-handlers) and
 * [Middleware](https://nextjs.org/docs/app/building-your-application/routing/middleware).
 *
 * Read more: [Next.js Docs: `headers`](https://nextjs.org/docs/app/api-reference/functions/headers)
 */
export declare function headers(): import("../../server/web/spec-extension/adapters/headers").ReadonlyHeaders;
export declare function cookies(): ReadonlyRequestCookies;
export declare function draftMode(): DraftMode;
