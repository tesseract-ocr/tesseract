import { actionAsyncStorage } from '../../server/app-render/action-async-storage.external';
import { RedirectStatusCode } from './redirect-status-code';
import { RedirectType, isRedirectError, REDIRECT_ERROR_CODE } from './redirect-error';
export function getRedirectError(url, type, statusCode) {
    if (statusCode === void 0) statusCode = RedirectStatusCode.TemporaryRedirect;
    const error = new Error(REDIRECT_ERROR_CODE);
    error.digest = REDIRECT_ERROR_CODE + ";" + type + ";" + url + ";" + statusCode + ";";
    return error;
}
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
 */ export function redirect(/** The URL to redirect to */ url, type) {
    const actionStore = actionAsyncStorage.getStore();
    const redirectType = type || ((actionStore == null ? void 0 : actionStore.isAction) ? RedirectType.push : RedirectType.replace);
    throw getRedirectError(url, redirectType, RedirectStatusCode.TemporaryRedirect);
}
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
 */ export function permanentRedirect(/** The URL to redirect to */ url, type) {
    if (type === void 0) type = RedirectType.replace;
    throw getRedirectError(url, type, RedirectStatusCode.PermanentRedirect);
}
export function getURLFromRedirectError(error) {
    if (!isRedirectError(error)) return null;
    // Slices off the beginning of the digest that contains the code and the
    // separating ';'.
    return error.digest.split(';').slice(2, -2).join(';');
}
export function getRedirectTypeFromError(error) {
    if (!isRedirectError(error)) {
        throw new Error('Not a redirect error');
    }
    return error.digest.split(';', 2)[1];
}
export function getRedirectStatusCodeFromError(error) {
    if (!isRedirectError(error)) {
        throw new Error('Not a redirect error');
    }
    return Number(error.digest.split(';').at(-2));
}

//# sourceMappingURL=redirect.js.map