declare global {
    var __next_log_error__: undefined | ((err: unknown) => void);
}
export type ErrorHandler = (err: unknown, errorInfo: unknown) => string | undefined;
export declare const ErrorHandlerSource: {
    readonly serverComponents: "serverComponents";
    readonly flightData: "flightData";
    readonly html: "html";
};
/**
 * Create error handler for renderers.
 * Tolerate dynamic server errors during prerendering so console
 * isn't spammed with unactionable errors
 */
export declare function createErrorHandler({ 
/**
 * Used for debugging
 */
source, dev, isNextExport, errorLogger, digestErrorsMap, allCapturedErrors, silenceLogger, }: {
    source: (typeof ErrorHandlerSource)[keyof typeof ErrorHandlerSource];
    dev?: boolean;
    isNextExport?: boolean;
    errorLogger?: (err: any) => Promise<void>;
    digestErrorsMap: Map<string, Error>;
    allCapturedErrors?: Error[];
    silenceLogger?: boolean;
}): ErrorHandler;
