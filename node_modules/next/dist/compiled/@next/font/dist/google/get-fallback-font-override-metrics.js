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
exports.getFallbackFontOverrideMetrics = getFallbackFontOverrideMetrics;
// @ts-ignore
const font_utils_1 = require("next/dist/server/font-utils");
// @ts-ignore
const Log = __importStar(require("next/dist/build/output/log"));
/**
 * Get precalculated fallback font metrics for the Google Fonts family.
 *
 * TODO:
 * We might want to calculate these values with fontkit instead (like in next/font/local).
 * That way we don't have to update the precalculated values every time a new font is added to Google Fonts.
 */
function getFallbackFontOverrideMetrics(fontFamily) {
    try {
        const { ascent, descent, lineGap, fallbackFont, sizeAdjust } = (0, font_utils_1.calculateSizeAdjustValues)(fontFamily);
        return {
            fallbackFont,
            ascentOverride: `${ascent}%`,
            descentOverride: `${descent}%`,
            lineGapOverride: `${lineGap}%`,
            sizeAdjust: `${sizeAdjust}%`,
        };
    }
    catch {
        Log.error(`Failed to find font override values for font \`${fontFamily}\``);
    }
}
