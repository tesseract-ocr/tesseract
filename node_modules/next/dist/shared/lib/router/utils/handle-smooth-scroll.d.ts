/**
 * Run function with `scroll-behavior: auto` applied to `<html/>`.
 * This css change will be reverted after the function finishes.
 */
export declare function handleSmoothScroll(fn: () => void, options?: {
    dontForceLayout?: boolean;
    onlyHashChange?: boolean;
}): void;
