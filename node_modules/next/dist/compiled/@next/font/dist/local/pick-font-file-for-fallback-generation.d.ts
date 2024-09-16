/**
 * If multiple font files are provided for a font family, we need to pick one to use for the automatic fallback generation.
 * This function returns the font file that is most likely to be used for the bulk of the text on a page.
 *
 * There are some assumptions here about the text on a page when picking the font file:
 * - Most of the text will have normal weight, use the one closest to 400
 * - Most of the text will have normal style, prefer normal over italic
 * - If two font files have the same distance from normal weight, the thinner one will most likely be the bulk of the text
 */
export declare function pickFontFileForFallbackGeneration<T extends {
    style?: string;
    weight?: string;
}>(fontFiles: T[]): T;
