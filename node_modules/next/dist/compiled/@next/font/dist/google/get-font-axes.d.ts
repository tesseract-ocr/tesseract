/**
 * Validates and gets the data for each font axis required to generate the Google Fonts URL.
 */
export declare function getFontAxes(fontFamily: string, weights: string[], styles: string[], selectedVariableAxes?: string[]): {
    wght?: string[];
    ital?: string[];
    variableAxes?: [string, string][];
};
