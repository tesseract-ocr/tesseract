import type { ErrorInfo } from 'react';
declare global {
    var __next_log_error__: undefined | ((err: unknown) => void);
}
type RSCErrorHandler = (err: unknown) => string | undefined;
type SSRErrorHandler = (err: unknown, errorInfo?: ErrorInfo) => string | undefined;
export type DigestedError = Error & {
    digest: string;
};
/**
 * Returns a digest for well-known Next.js errors, otherwise `undefined`. If a
 * digest is returned this also means that the error does not need to be
 * reported.
 */
export declare function getDigestForWellKnownError(error: unknown): string | undefined;
export declare function createFlightReactServerErrorHandler(shouldFormatError: boolean, onReactServerRenderError: (err: DigestedError) => void): RSCErrorHandler;
export declare function createHTMLReactServerErrorHandler(shouldFormatError: boolean, isNextExport: boolean, reactServerErrors: Map<string, DigestedError>, silenceLogger: boolean, onReactServerRenderError: undefined | ((err: DigestedError) => void)): RSCErrorHandler;
export declare function createHTMLErrorHandler(shouldFormatError: boolean, isNextExport: boolean, reactServerErrors: Map<string, DigestedError>, allCapturedErrors: Array<unknown>, silenceLogger: boolean, onHTMLRenderSSRError: (err: DigestedError, errorInfo?: ErrorInfo) => void): SSRErrorHandler;
export declare function isUserLandError(err: any): boolean;
export {};
