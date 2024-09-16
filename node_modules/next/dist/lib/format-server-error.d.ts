/**
 * Input:
 * Error: Something went wrong
    at funcName (/path/to/file.js:10:5)
    at anotherFunc (/path/to/file.js:15:10)
 
 * Output:
    at funcName (/path/to/file.js:10:5)
    at anotherFunc (/path/to/file.js:15:10)
 */
export declare function getStackWithoutErrorMessage(error: Error): string;
export declare function formatServerError(error: Error): void;
