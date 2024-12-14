import { isHTTPAccessFallbackError } from './http-access-fallback/http-access-fallback';
import { isRedirectError } from './redirect-error';
/**
 * Returns true if the error is a navigation signal error. These errors are
 * thrown by user code to perform navigation operations and interrupt the React
 * render.
 */ export function isNextRouterError(error) {
    return isRedirectError(error) || isHTTPAccessFallbackError(error);
}

//# sourceMappingURL=is-next-router-error.js.map