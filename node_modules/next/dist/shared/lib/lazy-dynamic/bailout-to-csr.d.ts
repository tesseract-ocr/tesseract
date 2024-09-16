/** An error that should be thrown when we want to bail out to client-side rendering. */
export declare class BailoutToCSRError extends Error {
    readonly reason: string;
    readonly digest = "BAILOUT_TO_CLIENT_SIDE_RENDERING";
    constructor(reason: string);
}
/** Checks if a passed argument is an error that is thrown if we want to bail out to client-side rendering. */
export declare function isBailoutToCSRError(err: unknown): err is BailoutToCSRError;
