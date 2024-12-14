import type { OutgoingHttpHeaders } from 'http';
/**
 * Converts a Node.js IncomingHttpHeaders object to a Headers object. Any
 * headers with multiple values will be joined with a comma and space. Any
 * headers that have an undefined value will be ignored and others will be
 * coerced to strings.
 *
 * @param nodeHeaders the headers object to convert
 * @returns the converted headers object
 */
export declare function fromNodeOutgoingHttpHeaders(nodeHeaders: OutgoingHttpHeaders): Headers;
export declare function splitCookiesString(cookiesString: string): string[];
/**
 * Converts a Headers object to a Node.js OutgoingHttpHeaders object. This is
 * required to support the set-cookie header, which may have multiple values.
 *
 * @param headers the headers object to convert
 * @returns the converted headers object
 */
export declare function toNodeOutgoingHttpHeaders(headers: Headers): OutgoingHttpHeaders;
/**
 * Validate the correctness of a user-provided URL.
 */
export declare function validateURL(url: string | URL): string;
/**
 * Normalizes `nxtP` and `nxtI` query param values to remove the prefix.
 * This function does not mutate the input key; it calls the provided function
 * with the normalized key.
 */
export declare function normalizeNextQueryParam(key: string, onKeyNormalized: (normalizedKey: string) => void): void;
