/**
 * Generate the Google Fonts URL given the requested weight(s), style(s) and additional variable axes
 */
export declare function getGoogleFontsUrl(fontFamily: string, axes: {
    wght?: string[];
    ital?: string[];
    variableAxes?: [string, string][];
}, display: string): string;
