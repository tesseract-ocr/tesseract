"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.validateGoogleFontFunctionCall = void 0;
const constants_1 = require("../constants");
const format_available_values_1 = require("../format-available-values");
const next_font_error_1 = require("../next-font-error");
const google_fonts_metadata_1 = require("./google-fonts-metadata");
/**
 * Validate the data recieved from next-swc next-transform-font on next/font/google calls
 */
function validateGoogleFontFunctionCall(functionName, fontFunctionArgument) {
    let { weight, style, preload = true, display = 'swap', axes, fallback, adjustFontFallback = true, variable, subsets, } = fontFunctionArgument || {};
    if (functionName === '') {
        (0, next_font_error_1.nextFontError)(`next/font/google has no default export`);
    }
    const fontFamily = functionName.replace(/_/g, ' ');
    // Get the Google font metadata, we'll use this to validate the font arguments and to print better error messages
    const fontFamilyData = google_fonts_metadata_1.googleFontsMetadata[fontFamily];
    if (!fontFamilyData) {
        (0, next_font_error_1.nextFontError)(`Unknown font \`${fontFamily}\``);
    }
    const availableSubsets = fontFamilyData.subsets;
    if (availableSubsets.length === 0) {
        // If the font doesn't have any preloadeable subsets, disable preload
        preload = false;
    }
    else if (preload) {
        if (!subsets) {
            (0, next_font_error_1.nextFontError)(`Preload is enabled but no subsets were specified for font \`${fontFamily}\`. Please specify subsets or disable preloading if your intended subset can't be preloaded.\nAvailable subsets: ${(0, format_available_values_1.formatAvailableValues)(availableSubsets)}\n\nRead more: https://nextjs.org/docs/messages/google-fonts-missing-subsets`);
        }
        subsets.forEach((subset) => {
            if (!availableSubsets.includes(subset)) {
                (0, next_font_error_1.nextFontError)(`Unknown subset \`${subset}\` for font \`${fontFamily}\`.\nAvailable subsets: ${(0, format_available_values_1.formatAvailableValues)(availableSubsets)}`);
            }
        });
    }
    const fontWeights = fontFamilyData.weights;
    const fontStyles = fontFamilyData.styles;
    // Get the unique weights and styles from the function call
    const weights = !weight
        ? []
        : [...new Set(Array.isArray(weight) ? weight : [weight])];
    const styles = !style
        ? []
        : [...new Set(Array.isArray(style) ? style : [style])];
    if (weights.length === 0) {
        // Set variable as default, throw if not available
        if (fontWeights.includes('variable')) {
            weights.push('variable');
        }
        else {
            (0, next_font_error_1.nextFontError)(`Missing weight for font \`${fontFamily}\`.\nAvailable weights: ${(0, format_available_values_1.formatAvailableValues)(fontWeights)}`);
        }
    }
    if (weights.length > 1 && weights.includes('variable')) {
        (0, next_font_error_1.nextFontError)(`Unexpected \`variable\` in weight array for font \`${fontFamily}\`. You only need \`variable\`, it includes all available weights.`);
    }
    weights.forEach((selectedWeight) => {
        if (!fontWeights.includes(selectedWeight)) {
            (0, next_font_error_1.nextFontError)(`Unknown weight \`${selectedWeight}\` for font \`${fontFamily}\`.\nAvailable weights: ${(0, format_available_values_1.formatAvailableValues)(fontWeights)}`);
        }
    });
    if (styles.length === 0) {
        if (fontStyles.length === 1) {
            // Handle default style for fonts that only have italic
            styles.push(fontStyles[0]);
        }
        else {
            // Otherwise set default style to normal
            styles.push('normal');
        }
    }
    styles.forEach((selectedStyle) => {
        if (!fontStyles.includes(selectedStyle)) {
            (0, next_font_error_1.nextFontError)(`Unknown style \`${selectedStyle}\` for font \`${fontFamily}\`.\nAvailable styles: ${(0, format_available_values_1.formatAvailableValues)(fontStyles)}`);
        }
    });
    if (!constants_1.allowedDisplayValues.includes(display)) {
        (0, next_font_error_1.nextFontError)(`Invalid display value \`${display}\` for font \`${fontFamily}\`.\nAvailable display values: ${(0, format_available_values_1.formatAvailableValues)(constants_1.allowedDisplayValues)}`);
    }
    if (weights[0] !== 'variable' && axes) {
        (0, next_font_error_1.nextFontError)('Axes can only be defined for variable fonts');
    }
    return {
        fontFamily,
        weights,
        styles,
        display,
        preload,
        selectedVariableAxes: axes,
        fallback,
        adjustFontFallback,
        variable,
        subsets,
    };
}
exports.validateGoogleFontFunctionCall = validateGoogleFontFunctionCall;
