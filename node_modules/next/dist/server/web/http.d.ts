/**
 * List of valid HTTP methods that can be implemented by Next.js's Custom App
 * Routes.
 */
export declare const HTTP_METHODS: readonly ["GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE", "PATCH"];
/**
 * A type representing the valid HTTP methods that can be implemented by
 * Next.js's Custom App Routes.
 */
export type HTTP_METHOD = (typeof HTTP_METHODS)[number];
/**
 * Checks to see if the passed string is an HTTP method. Note that this is case
 * sensitive.
 *
 * @param maybeMethod the string that may be an HTTP method
 * @returns true if the string is an HTTP method
 */
export declare function isHTTPMethod(maybeMethod: string): maybeMethod is HTTP_METHOD;
