"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "calculateSizeAdjustValues", {
    enumerable: true,
    get: function() {
        return calculateSizeAdjustValues;
    }
});
const _constants = require("../shared/lib/constants");
const capsizeFontsMetrics = require('next/dist/server/capsize-font-metrics.json');
function formatName(str) {
    return str.replace(/(?:^\w|[A-Z]|\b\w)/g, function(word, index) {
        return index === 0 ? word.toLowerCase() : word.toUpperCase();
    }).replace(/\s+/g, '');
}
function formatOverrideValue(val) {
    return Math.abs(val * 100).toFixed(2);
}
function calculateSizeAdjustValues(fontName) {
    const fontKey = formatName(fontName);
    const fontMetrics = capsizeFontsMetrics[fontKey];
    let { category, ascent, descent, lineGap, unitsPerEm, xWidthAvg } = fontMetrics;
    const mainFontAvgWidth = xWidthAvg / unitsPerEm;
    const fallbackFont = category === 'serif' ? _constants.DEFAULT_SERIF_FONT : _constants.DEFAULT_SANS_SERIF_FONT;
    const fallbackFontName = formatName(fallbackFont.name);
    const fallbackFontMetrics = capsizeFontsMetrics[fallbackFontName];
    const fallbackFontAvgWidth = fallbackFontMetrics.xWidthAvg / fallbackFontMetrics.unitsPerEm;
    let sizeAdjust = xWidthAvg ? mainFontAvgWidth / fallbackFontAvgWidth : 1;
    ascent = formatOverrideValue(ascent / (unitsPerEm * sizeAdjust));
    descent = formatOverrideValue(descent / (unitsPerEm * sizeAdjust));
    lineGap = formatOverrideValue(lineGap / (unitsPerEm * sizeAdjust));
    return {
        ascent,
        descent,
        lineGap,
        fallbackFont: fallbackFont.name,
        sizeAdjust: formatOverrideValue(sizeAdjust)
    };
}

//# sourceMappingURL=font-utils.js.map