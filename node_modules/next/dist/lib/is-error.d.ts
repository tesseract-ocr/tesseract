export interface NextError extends Error {
    type?: string;
    page?: string;
    code?: string | number;
    cancelled?: boolean;
    digest?: number;
}
/**
 * Checks whether the given value is a NextError.
 * This can be used to print a more detailed error message with properties like `code` & `digest`.
 */
export default function isError(err: unknown): err is NextError;
export declare function getProperError(err: unknown): Error;
