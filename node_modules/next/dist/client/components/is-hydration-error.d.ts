export declare const getDefaultHydrationErrorMessage: () => string;
export declare function isHydrationError(error: unknown): boolean;
export declare function isReactHydrationErrorMessage(msg: string): boolean;
export declare function getHydrationErrorStackInfo(rawMessage: string): {
    message: string | null;
    link?: string;
    stack?: string;
    diff?: string;
};
