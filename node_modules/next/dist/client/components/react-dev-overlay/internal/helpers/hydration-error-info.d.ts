export type HydrationErrorState = {
    warning?: [string, string, string];
    componentStack?: string;
    serverContent?: string;
    clientContent?: string;
    notes?: string;
    reactOutputComponentDiff?: string;
};
type NullableText = string | null | undefined;
export declare const hydrationErrorState: HydrationErrorState;
export declare const getHydrationWarningType: (message: NullableText) => "tag" | "text" | "text-in-tag";
export declare const getReactHydrationDiffSegments: (msg: NullableText) => (string | undefined)[] | undefined;
/**
 * Patch console.error to capture hydration errors.
 * If any of the knownHydrationWarnings are logged, store the message and component stack.
 * When the hydration runtime error is thrown, the message and component stack are added to the error.
 * This results in a more helpful error message in the error overlay.
 */
export declare function storeHydrationErrorStateFromConsoleArgs(...args: any[]): void;
export {};
