export type ErrorHandler = (error: Error) => void;
export declare function handleClientError(originError: unknown, consoleErrorArgs: any[], capturedFromConsole?: boolean): void;
export declare function useErrorHandler(handleOnUnhandledError: ErrorHandler, handleOnUnhandledRejection: ErrorHandler): void;
export declare function handleGlobalErrors(): void;
