/**
 * List of valid HTTP methods that can be implemented by Next.js's Custom App
 * Routes.
 */ export const HTTP_METHODS = [
    'GET',
    'HEAD',
    'OPTIONS',
    'POST',
    'PUT',
    'DELETE',
    'PATCH'
];
/**
 * Checks to see if the passed string is an HTTP method. Note that this is case
 * sensitive.
 *
 * @param maybeMethod the string that may be an HTTP method
 * @returns true if the string is an HTTP method
 */ export function isHTTPMethod(maybeMethod) {
    return HTTP_METHODS.includes(maybeMethod);
}

//# sourceMappingURL=http.js.map