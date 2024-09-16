export declare class StaticGenBailoutError extends Error {
    readonly code = "NEXT_STATIC_GEN_BAILOUT";
}
export declare function isStaticGenBailoutError(error: unknown): error is StaticGenBailoutError;
