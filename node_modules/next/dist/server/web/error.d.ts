export declare class PageSignatureError extends Error {
    constructor({ page }: {
        page: string;
    });
}
export declare class RemovedPageError extends Error {
    constructor();
}
export declare class RemovedUAError extends Error {
    constructor();
}
