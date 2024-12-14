import type { IncomingHttpHeaders } from 'node:http';
/**
 * Removes the flight headers from the request.
 *
 * @param req the request to strip the headers from
 */
export declare function stripFlightHeaders(headers: IncomingHttpHeaders): void;
