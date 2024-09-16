type FontOptions = {
    src: Array<{
        path: string;
        weight?: string;
        style?: string;
        ext: string;
        format: string;
    }>;
    display: string;
    weight?: string;
    style?: string;
    fallback?: string[];
    preload: boolean;
    variable?: string;
    adjustFontFallback?: string | false;
    declarations?: Array<{
        prop: string;
        value: string;
    }>;
};
/**
 * Validate the data recieved from next-swc next-transform-font on next/font/local calls
 */
export declare function validateLocalFontFunctionCall(functionName: string, fontData: any): FontOptions;
export {};
