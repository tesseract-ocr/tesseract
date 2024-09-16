/// <reference types="node" />
import type { IncomingMessage } from 'http';
import type { BaseNextRequest } from '../base-http';
import type { NextApiResponse } from '../../shared/lib/utils';
export type NextApiRequestCookies = Partial<{
    [key: string]: string;
}>;
export type NextApiRequestQuery = Partial<{
    [key: string]: string | string[];
}>;
export type __ApiPreviewProps = {
    previewModeId: string;
    previewModeEncryptionKey: string;
    previewModeSigningKey: string;
};
export declare function wrapApiHandler<T extends (...args: any[]) => any>(page: string, handler: T): T;
/**
 *
 * @param res response object
 * @param statusCode `HTTP` status code of response
 */
export declare function sendStatusCode(res: NextApiResponse, statusCode: number): NextApiResponse<any>;
/**
 *
 * @param res response object
 * @param [statusOrUrl] `HTTP` status code of redirect
 * @param url URL of redirect
 */
export declare function redirect(res: NextApiResponse, statusOrUrl: string | number, url?: string): NextApiResponse<any>;
export declare function checkIsOnDemandRevalidate(req: Request | IncomingMessage | BaseNextRequest, previewProps: __ApiPreviewProps): {
    isOnDemandRevalidate: boolean;
    revalidateOnlyGenerated: boolean;
};
export declare const COOKIE_NAME_PRERENDER_BYPASS = "__prerender_bypass";
export declare const COOKIE_NAME_PRERENDER_DATA = "__next_preview_data";
export declare const RESPONSE_LIMIT_DEFAULT: number;
export declare const SYMBOL_PREVIEW_DATA: unique symbol;
export declare const SYMBOL_CLEARED_COOKIES: unique symbol;
export declare function clearPreviewData<T>(res: NextApiResponse<T>, options?: {
    path?: string;
}): NextApiResponse<T>;
/**
 * Custom error class
 */
export declare class ApiError extends Error {
    readonly statusCode: number;
    constructor(statusCode: number, message: string);
}
/**
 * Sends error in `response`
 * @param res response object
 * @param statusCode of response
 * @param message of response
 */
export declare function sendError(res: NextApiResponse, statusCode: number, message: string): void;
interface LazyProps {
    req: IncomingMessage;
}
/**
 * Execute getter function only if its needed
 * @param LazyProps `req` and `params` for lazyProp
 * @param prop name of property
 * @param getter function to get data
 */
export declare function setLazyProp<T>({ req }: LazyProps, prop: string, getter: () => T): void;
export {};
