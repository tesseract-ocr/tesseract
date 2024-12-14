"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getFontAxes = getFontAxes;
const format_available_values_1 = require("../format-available-values");
const next_font_error_1 = require("../next-font-error");
const google_fonts_metadata_1 = require("./google-fonts-metadata");
/**
 * Validates and gets the data for each font axis required to generate the Google Fonts URL.
 */
function getFontAxes(fontFamily, weights, styles, selectedVariableAxes) {
    const hasItalic = styles.includes('italic');
    const hasNormal = styles.includes('normal');
    // Make sure the order is correct, otherwise Google Fonts will return an error
    // If only normal is set, we can skip returning the ital axis as normal is the default
    const ital = hasItalic ? [...(hasNormal ? ['0'] : []), '1'] : undefined;
    // Weights will always contain one element if it's a variable font
    if (weights[0] === 'variable') {
        // Get all the available axes for the current font from the metadata file
        const allAxes = google_fonts_metadata_1.googleFontsMetadata[fontFamily].axes;
        if (!allAxes) {
            throw new Error('invariant variable font without axes');
        }
        if (selectedVariableAxes) {
            // The axes other than weight and style that can be defined for the current variable font
            const defineAbleAxes = allAxes
                .map(({ tag }) => tag)
                .filter((tag) => tag !== 'wght');
            if (defineAbleAxes.length === 0) {
                (0, next_font_error_1.nextFontError)(`Font \`${fontFamily}\` has no definable \`axes\``);
            }
            if (!Array.isArray(selectedVariableAxes)) {
                (0, next_font_error_1.nextFontError)(`Invalid axes value for font \`${fontFamily}\`, expected an array of axes.\nAvailable axes: ${(0, format_available_values_1.formatAvailableValues)(defineAbleAxes)}`);
            }
            selectedVariableAxes.forEach((key) => {
                if (!defineAbleAxes.some((tag) => tag === key)) {
                    (0, next_font_error_1.nextFontError)(`Invalid axes value \`${key}\` for font \`${fontFamily}\`.\nAvailable axes: ${(0, format_available_values_1.formatAvailableValues)(defineAbleAxes)}`);
                }
            });
        }
        let weightAxis;
        let variableAxes;
        for (const { tag, min, max } of allAxes) {
            if (tag === 'wght') {
                // In variable fonts the weight is a range
                weightAxis = `${min}..${max}`;
            }
            else if (selectedVariableAxes === null || selectedVariableAxes === void 0 ? void 0 : selectedVariableAxes.includes(tag)) {
                if (!variableAxes) {
                    variableAxes = [];
                }
                variableAxes.push([tag, `${min}..${max}`]);
            }
        }
        return {
            wght: weightAxis ? [weightAxis] : undefined,
            ital,
            variableAxes,
        };
    }
    else {
        return {
            ital,
            wght: weights,
        };
    }
}
