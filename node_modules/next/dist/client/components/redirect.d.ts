import { RedirectStatusCode } from './redirect-status-code';
import { RedirectType, type RedirectError } from './redirect-error';
export declare function getRedirectError(url: string, type: RedirectType, statusCode?: RedirectStatusCode): RedirectError;
/**
 * This function allows you to redirect the user to another URL. It can be used in
 * [Server Components](https://nextjs.org/docs/app/building-your-application/rendering/server-components),
 * [Route Handlers](https://nextjs.org/docs/app/building-your-application/routing/route-handlers), and
 * [Server Actions](https://nextjs.org/docs/app/building-your-application/data-fetching/server-actions-and-mutations).
 *
 * - In a Server Component, this will insert a meta tag to redirect the user to the target page.
 * - In a Route Handler or Server Action, it will serve a 307/303 to the caller.
 * - In a Server Action, type defaults to 'push' and 'replace' elsewhere.
 *
 * Read more: [Next.js Docs: `redirect`](https://nextjs.org/docs/app/api-reference/functions/redirect)
 */
export declare function redirect(
/** The URL to redirect to */
url: string, type?: RedirectType): never;
/**
 * This function allows you to redirect the user to another URL. It can be used in
 * [Server Components](https://nextjs.org/docs/app/building-your-application/rendering/server-components),
 * [Route Handlers](https://nextjs.org/docs/app/building-your-application/routing/route-handlers), and
 * [Server Actions](https://nextjs.org/docs/app/building-your-application/data-fetching/server-actions-and-mutations).
 *
 * - In a Server Component, this will insert a meta tag to redirect the user to the target page.
 * - In a Route Handler or Server Action, it will serve a 308/303 to the caller.
 *
 * Read more: [Next.js Docs: `redirect`](https://nextjs.org/docs/app/api-reference/functions/redirect)
 */
export declare function permanentRedirect(
/** The URL to redirect to */
url: string, type?: RedirectType): never;
/**
 * Returns the encoded URL from the error if it's a RedirectError, null
 * otherwise. Note that this does not validate the URL returned.
 *
 * @param error the error that may be a redirect error
 * @return the url if the error was a redirect error
 */
export declare function getURLFromRedirectError(error: RedirectError): string;
export declare function getRedirectTypeFromError(error: RedirectError): RedirectType;
export declare function getRedirectStatusCodeFromError(error: RedirectError): number;
