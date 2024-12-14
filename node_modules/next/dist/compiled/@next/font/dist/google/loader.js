"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
// @ts-ignore
const Log = __importStar(require("next/dist/build/output/log"));
const validate_google_font_function_call_1 = require("./validate-google-font-function-call");
const get_font_axes_1 = require("./get-font-axes");
const get_google_fonts_url_1 = require("./get-google-fonts-url");
const next_font_error_1 = require("../next-font-error");
const find_font_files_in_css_1 = require("./find-font-files-in-css");
const get_fallback_font_override_metrics_1 = require("./get-fallback-font-override-metrics");
const fetch_css_from_google_fonts_1 = require("./fetch-css-from-google-fonts");
const fetch_font_file_1 = require("./fetch-font-file");
const cssCache = new Map();
const fontCache = new Map();
// regexp is based on https://github.com/sindresorhus/escape-string-regexp
const reHasRegExp = /[|\\{}()[\]^$+*?.-]/;
const reReplaceRegExp = /[|\\{}()[\]^$+*?.-]/g;
function escapeStringRegexp(str) {
    // see also: https://github.com/lodash/lodash/blob/2da024c3b4f9947a48517639de7560457cd4ec6c/escapeRegExp.js#L23
    if (reHasRegExp.test(str)) {
        return str.replace(reReplaceRegExp, '\\$&');
    }
    return str;
}
const nextFontGoogleFontLoader = async ({ functionName, data, emitFontFile, isDev, isServer, }) => {
    var _a;
    const { fontFamily, weights, styles, display, preload, selectedVariableAxes, fallback, adjustFontFallback, variable, subsets, } = (0, validate_google_font_function_call_1.validateGoogleFontFunctionCall)(functionName, data[0]);
    // Validate and get the font axes required to generated the URL
    const fontAxes = (0, get_font_axes_1.getFontAxes)(fontFamily, weights, styles, selectedVariableAxes);
    // Generate the Google Fonts URL from the font family, axes and display value
    const url = (0, get_google_fonts_url_1.getGoogleFontsUrl)(fontFamily, fontAxes, display);
    // Get precalculated fallback font metrics, used to generate the fallback font CSS
    const adjustFontFallbackMetrics = adjustFontFallback ? (0, get_fallback_font_override_metrics_1.getFallbackFontOverrideMetrics)(fontFamily) : undefined;
    const result = {
        fallbackFonts: fallback,
        weight: weights.length === 1 && weights[0] !== 'variable'
            ? weights[0]
            : undefined,
        style: styles.length === 1 ? styles[0] : undefined,
        variable,
        adjustFontFallback: adjustFontFallbackMetrics,
    };
    try {
        /**
         * Hacky way to make sure the fetch is only done once.
         * Otherwise both the client and server compiler would fetch the CSS.
         * The reason we need to return the actual CSS from both the server and client is because a hash is generated based on the CSS content.
         */
        const hasCachedCSS = cssCache.has(url);
        // Fetch CSS from Google Fonts or get it from the cache
        let fontFaceDeclarations = hasCachedCSS
            ? cssCache.get(url)
            : await (0, fetch_css_from_google_fonts_1.fetchCSSFromGoogleFonts)(url, fontFamily, isDev).catch((err) => {
                console.error(err);
                return null;
            });
        if (!hasCachedCSS) {
            cssCache.set(url, fontFaceDeclarations !== null && fontFaceDeclarations !== void 0 ? fontFaceDeclarations : null);
        }
        else {
            cssCache.delete(url);
        }
        if (fontFaceDeclarations == null) {
            (0, next_font_error_1.nextFontError)(`Failed to fetch \`${fontFamily}\` from Google Fonts.`);
        }
        // CSS Variables may be set on a body tag, ignore them to keep the CSS module pure
        fontFaceDeclarations = fontFaceDeclarations.split('body {', 1)[0];
        // Find font files to download, provide the array of subsets we want to preload if preloading is enabled
        const fontFiles = (0, find_font_files_in_css_1.findFontFilesInCss)(fontFaceDeclarations, preload ? subsets : undefined);
        // Download the font files extracted from the CSS
        const downloadedFiles = await Promise.all(fontFiles.map(async ({ googleFontFileUrl, preloadFontFile }) => {
            const hasCachedFont = fontCache.has(googleFontFileUrl);
            // Download the font file or get it from cache
            const fontFileBuffer = hasCachedFont
                ? fontCache.get(googleFontFileUrl)
                : await (0, fetch_font_file_1.fetchFontFile)(googleFontFileUrl, isDev).catch((err) => {
                    console.error(err);
                    return null;
                });
            if (!hasCachedFont) {
                fontCache.set(googleFontFileUrl, fontFileBuffer !== null && fontFileBuffer !== void 0 ? fontFileBuffer : null);
            }
            else {
                fontCache.delete(googleFontFileUrl);
            }
            if (fontFileBuffer == null) {
                (0, next_font_error_1.nextFontError)(`Failed to fetch \`${fontFamily}\` from Google Fonts.`);
            }
            const ext = /\.(woff|woff2|eot|ttf|otf)$/.exec(googleFontFileUrl)[1];
            // Emit font file to .next/static/media
            const selfHostedFileUrl = emitFontFile(fontFileBuffer, ext, preloadFontFile, !!adjustFontFallbackMetrics);
            return {
                googleFontFileUrl,
                selfHostedFileUrl,
            };
        }));
        /**
         * Replace the @font-face sources with the self-hosted files we just downloaded to .next/static/media
         *
         * E.g.
         * @font-face {
         *   font-family: 'Inter';
         *   src: url(https://fonts.gstatic.com/...) -> url(/_next/static/media/_.woff2)
         * }
         */
        let updatedCssResponse = fontFaceDeclarations;
        for (const { googleFontFileUrl, selfHostedFileUrl } of downloadedFiles) {
            updatedCssResponse = updatedCssResponse.replace(new RegExp(escapeStringRegexp(googleFontFileUrl), 'g'), selfHostedFileUrl);
        }
        return {
            ...result,
            css: updatedCssResponse,
        };
    }
    catch (err) {
        if (isDev) {
            if (isServer) {
                Log.error(`Failed to download \`${fontFamily}\` from Google Fonts. Using fallback font instead.\n\n${err.message}}`);
            }
            // In dev we should return the fallback font instead of throwing an error
            let css = `@font-face {
  font-family: '${fontFamily} Fallback';
  src: local("${(_a = adjustFontFallbackMetrics === null || adjustFontFallbackMetrics === void 0 ? void 0 : adjustFontFallbackMetrics.fallbackFont) !== null && _a !== void 0 ? _a : 'Arial'}");`;
            if (adjustFontFallbackMetrics) {
                css += `
  ascent-override:${adjustFontFallbackMetrics.ascentOverride};
  descent-override:${adjustFontFallbackMetrics.descentOverride};
  line-gap-override:${adjustFontFallbackMetrics.lineGapOverride};
  size-adjust:${adjustFontFallbackMetrics.sizeAdjust};`;
            }
            css += '\n}';
            return {
                ...result,
                css,
            };
        }
        else {
            throw err;
        }
    }
};
exports.default = nextFontGoogleFontLoader;
