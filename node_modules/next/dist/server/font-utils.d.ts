export type FontManifest = Array<{
    url: string;
    content: string;
}>;
export type FontConfig = boolean;
export declare function getFontDefinitionFromNetwork(url: string): Promise<string>;
export declare function calculateOverrideValues(fontName: string): {
    ascent: any;
    descent: any;
    lineGap: any;
    fallbackFont: string;
};
export declare function calculateSizeAdjustValues(fontName: string): {
    ascent: any;
    descent: any;
    lineGap: any;
    fallbackFont: string;
    sizeAdjust: string;
};
export declare function getFontOverrideCss(url: string, css: string, useSizeAdjust?: boolean): string;
