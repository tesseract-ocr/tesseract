import type { BaseNextRequest, BaseNextResponse } from './base-http';
/**
 * Sends the response on the underlying next response object.
 *
 * @param req the underlying request object
 * @param res the underlying response object
 * @param response the response to send
 */
export declare function sendResponse(req: BaseNextRequest, res: BaseNextResponse, response: Response, waitUntil?: Promise<unknown>): Promise<void>;
