import type { AdjustFontFallback } from '../../../../../font';
/**
 * The next/font postcss plugin recieves the @font-face declarations returned from the next/font loaders.
 *
 * It hashes the font-family name to make it unguessable, it shouldn't be globally accessible.
 * If it were global, we wouldn't be able to tell which pages are using which fonts when generating preload tags.
 *
 * If the font loader returned fallback metrics, generate a fallback @font-face.
 *
 * If the font loader returned a variable name, add a CSS class that declares a variable containing the font and fallback fonts.
 *
 * Lastly, it adds the font-family to the exports object.
 * This enables you to access the actual font-family name, not just through the CSS class.
 * e.g:
 * const inter = Inter({ subsets: ['latin'] })
 * inter.style.fontFamily // => '__Inter_123456'
 */
declare const postcssNextFontPlugin: {
    ({ exports, fontFamilyHash, fallbackFonts, adjustFontFallback, variable, weight, style, }: {
        exports: {
            name: any;
            value: any;
        }[];
        fontFamilyHash: string;
        fallbackFonts?: string[] | undefined;
        adjustFontFallback?: AdjustFontFallback | undefined;
        variable?: string | undefined;
        weight?: string | undefined;
        style?: string | undefined;
    }): {
        postcssPlugin: string;
        Once(root: any): void;
    };
    postcss: boolean;
};
export default postcssNextFontPlugin;
