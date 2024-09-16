import type { Font } from 'fontkit';
import type { AdjustFontFallback } from 'next/font';
/**
 * Given a font file and category, calculate the fallback font override values.
 * The returned values can be used to generate a CSS @font-face declaration.
 *
 * For example:
 * @font-face {
 *   font-family: local-font;
 *   src: local(Arial);
 *   size-adjust: 90%;
 * }
 *
 * Read more about this technique in these texts by the Google Aurora team:
 * https://developer.chrome.com/blog/font-fallbacks/
 * https://docs.google.com/document/d/e/2PACX-1vRsazeNirATC7lIj2aErSHpK26hZ6dA9GsQ069GEbq5fyzXEhXbvByoftSfhG82aJXmrQ_sJCPBqcx_/pub
 */
export declare function getFallbackMetricsFromFontFile(font: Font, category?: string): AdjustFontFallback;
