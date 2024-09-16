declare const DYNAMIC_ERROR_CODE = "DYNAMIC_SERVER_USAGE";
export declare class DynamicServerError extends Error {
    readonly description: string;
    digest: typeof DYNAMIC_ERROR_CODE;
    constructor(description: string);
}
export declare function isDynamicServerError(err: unknown): err is DynamicServerError;
export {};
