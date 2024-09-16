"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    calculateOverrideValues: null,
    calculateSizeAdjustValues: null,
    getFontDefinitionFromNetwork: null,
    getFontOverrideCss: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    calculateOverrideValues: function() {
        return calculateOverrideValues;
    },
    calculateSizeAdjustValues: function() {
        return calculateSizeAdjustValues;
    },
    getFontDefinitionFromNetwork: function() {
        return getFontDefinitionFromNetwork;
    },
    getFontOverrideCss: function() {
        return getFontOverrideCss;
    }
});
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _constants = require("../shared/lib/constants");
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const capsizeFontsMetrics = require("next/dist/server/capsize-font-metrics.json");
const CHROME_UA = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.61 Safari/537.36";
const IE_UA = "Mozilla/5.0 (Windows NT 10.0; Trident/7.0; rv:11.0) like Gecko";
function isGoogleFont(url) {
    return url.startsWith(_constants.GOOGLE_FONT_PROVIDER);
}
async function getFontForUA(url, UA) {
    const res = await fetch(url, {
        headers: {
            "user-agent": UA
        }
    });
    return await res.text();
}
async function getFontDefinitionFromNetwork(url) {
    let result = "";
    /**
   * The order of IE -> Chrome is important, other wise chrome starts loading woff1.
   * CSS cascading 🤷‍♂️.
   */ try {
        if (isGoogleFont(url)) {
            result += await getFontForUA(url, IE_UA);
        }
        result += await getFontForUA(url, CHROME_UA);
    } catch (e) {
        _log.warn(`Failed to download the stylesheet for ${url}. Skipped optimizing this font.`);
        return "";
    }
    return result;
}
function parseGoogleFontName(css) {
    const regex = /font-family: ([^;]*)/g;
    const matches = css.matchAll(regex);
    const fontNames = new Set();
    for (let font of matches){
        const fontFamily = font[1].replace(/^['"]|['"]$/g, "");
        fontNames.add(fontFamily);
    }
    return [
        ...fontNames
    ];
}
function formatName(str) {
    return str.replace(/(?:^\w|[A-Z]|\b\w)/g, function(word, index) {
        return index === 0 ? word.toLowerCase() : word.toUpperCase();
    }).replace(/\s+/g, "");
}
function formatOverrideValue(val) {
    return Math.abs(val * 100).toFixed(2);
}
function calculateOverrideValues(fontName) {
    const fontKey = formatName(fontName);
    const fontMetrics = capsizeFontsMetrics[fontKey];
    let { category, ascent, descent, lineGap, unitsPerEm } = fontMetrics;
    const fallbackFont = category === "serif" ? _constants.DEFAULT_SERIF_FONT : _constants.DEFAULT_SANS_SERIF_FONT;
    ascent = formatOverrideValue(ascent / unitsPerEm);
    descent = formatOverrideValue(descent / unitsPerEm);
    lineGap = formatOverrideValue(lineGap / unitsPerEm);
    return {
        ascent,
        descent,
        lineGap,
        fallbackFont: fallbackFont.name
    };
}
function calculateSizeAdjustValues(fontName) {
    const fontKey = formatName(fontName);
    const fontMetrics = capsizeFontsMetrics[fontKey];
    let { category, ascent, descent, lineGap, unitsPerEm, xWidthAvg } = fontMetrics;
    const mainFontAvgWidth = xWidthAvg / unitsPerEm;
    const fallbackFont = category === "serif" ? _constants.DEFAULT_SERIF_FONT : _constants.DEFAULT_SANS_SERIF_FONT;
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
function calculateOverrideCSS(font) {
    const fontName = font.trim();
    const { ascent, descent, lineGap, fallbackFont } = calculateOverrideValues(fontName);
    return `
    @font-face {
      font-family: "${fontName} Fallback";
      ascent-override: ${ascent}%;
      descent-override: ${descent}%;
      line-gap-override: ${lineGap}%;
      src: local("${fallbackFont}");
    }
  `;
}
function calculateSizeAdjustCSS(font) {
    const fontName = font.trim();
    const { ascent, descent, lineGap, fallbackFont, sizeAdjust } = calculateSizeAdjustValues(fontName);
    return `
    @font-face {
      font-family: "${fontName} Fallback";
      ascent-override: ${ascent}%;
      descent-override: ${descent}%;
      line-gap-override: ${lineGap}%;
      size-adjust: ${sizeAdjust}%;
      src: local("${fallbackFont}");
    }
  `;
}
function getFontOverrideCss(url, css, useSizeAdjust = false) {
    if (!isGoogleFont(url)) {
        return "";
    }
    const calcFn = useSizeAdjust ? calculateSizeAdjustCSS : calculateOverrideCSS;
    try {
        const fontNames = parseGoogleFontName(css);
        const fontCss = fontNames.reduce((cssStr, fontName)=>{
            cssStr += calcFn(fontName);
            return cssStr;
        }, "");
        return fontCss;
    } catch (e) {
        console.log("Error getting font override values - ", e);
        return "";
    }
}

//# sourceMappingURL=font-utils.js.map