/**
 * Get precalculated fallback font metrics for the Google Fonts family.
 *
 * TODO:
 * We might want to calculate these values with fontkit instead (like in next/font/local).
 * That way we don't have to update the precalculated values every time a new font is added to Google Fonts.
 */
export declare function getFallbackFontOverrideMetrics(fontFamily: string): {
    fallbackFont: any;
    ascentOverride: string;
    descentOverride: string;
    lineGapOverride: string;
    sizeAdjust: string;
} | undefined;
