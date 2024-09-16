type FontOptions = {
    fontFamily: string;
    weights: string[];
    styles: string[];
    display: string;
    preload: boolean;
    selectedVariableAxes?: string[];
    fallback?: string[];
    adjustFontFallback: boolean;
    variable?: string;
    subsets: string[];
};
/**
 * Validate the data recieved from next-swc next-transform-font on next/font/google calls
 */
export declare function validateGoogleFontFunctionCall(functionName: string, fontFunctionArgument: any): FontOptions;
export {};
