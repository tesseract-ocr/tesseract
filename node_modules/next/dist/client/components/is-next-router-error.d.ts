import { type HTTPAccessFallbackError } from './http-access-fallback/http-access-fallback';
import { type RedirectError } from './redirect-error';
/**
 * Returns true if the error is a navigation signal error. These errors are
 * thrown by user code to perform navigation operations and interrupt the React
 * render.
 */
export declare function isNextRouterError(error: unknown): error is RedirectError | HTTPAccessFallbackError;
