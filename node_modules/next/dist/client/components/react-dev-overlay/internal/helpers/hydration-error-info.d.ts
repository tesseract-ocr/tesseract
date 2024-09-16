export type HydrationErrorState = {
    warning?: [string, string, string];
    componentStack?: string;
    serverContent?: string;
    clientContent?: string;
};
type NullableText = string | null | undefined;
export declare const getHydrationWarningType: (msg: NullableText) => 'tag' | 'text' | 'text-in-tag';
export declare const hydrationErrorState: HydrationErrorState;
/**
 * Patch console.error to capture hydration errors.
 * If any of the knownHydrationWarnings are logged, store the message and component stack.
 * When the hydration runtime error is thrown, the message and component stack are added to the error.
 * This results in a more helpful error message in the error overlay.
 */
export declare function patchConsoleError(): void;
export {};
