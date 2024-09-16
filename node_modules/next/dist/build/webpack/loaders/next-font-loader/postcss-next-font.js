"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _postcss = /*#__PURE__*/ _interop_require_default(require("postcss"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
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
 */ const postcssNextFontPlugin = ({ exports: exports1, fontFamilyHash, fallbackFonts = [], adjustFontFallback, variable, weight, style })=>{
    return {
        postcssPlugin: "postcss-next-font",
        Once (root) {
            let fontFamily;
            const normalizeFamily = (family)=>{
                return family.replace(/['"]/g, "");
            };
            const formatFamily = (family)=>{
                // Turn the font family unguessable to make it locally scoped
                return `'__${family.replace(/ /g, "_")}_${fontFamilyHash}'`;
            };
            // Hash font-family names
            for (const node of root.nodes){
                if (node.type === "atrule" && node.name === "font-face") {
                    const familyNode = node.nodes.find((decl)=>decl.prop === "font-family");
                    if (!familyNode) {
                        continue;
                    }
                    if (!fontFamily) {
                        fontFamily = normalizeFamily(familyNode.value);
                    }
                    familyNode.value = formatFamily(fontFamily);
                }
            }
            if (!fontFamily) {
                throw new Error("Font loaders must return one or more @font-face's");
            }
            // Add fallback @font-face with the provided override values
            let adjustFontFallbackFamily;
            if (adjustFontFallback) {
                adjustFontFallbackFamily = formatFamily(`${fontFamily} Fallback`);
                const fallbackFontFace = _postcss.default.atRule({
                    name: "font-face"
                });
                const { fallbackFont, ascentOverride, descentOverride, lineGapOverride, sizeAdjust } = adjustFontFallback;
                fallbackFontFace.nodes = [
                    new _postcss.default.Declaration({
                        prop: "font-family",
                        value: adjustFontFallbackFamily
                    }),
                    new _postcss.default.Declaration({
                        prop: "src",
                        value: `local("${fallbackFont}")`
                    }),
                    ...ascentOverride ? [
                        new _postcss.default.Declaration({
                            prop: "ascent-override",
                            value: ascentOverride
                        })
                    ] : [],
                    ...descentOverride ? [
                        new _postcss.default.Declaration({
                            prop: "descent-override",
                            value: descentOverride
                        })
                    ] : [],
                    ...lineGapOverride ? [
                        new _postcss.default.Declaration({
                            prop: "line-gap-override",
                            value: lineGapOverride
                        })
                    ] : [],
                    ...sizeAdjust ? [
                        new _postcss.default.Declaration({
                            prop: "size-adjust",
                            value: sizeAdjust
                        })
                    ] : []
                ];
                root.nodes.push(fallbackFontFace);
            }
            // Variable fonts can define ranges of values
            const isRange = (value)=>value.trim().includes(" ");
            // Format the font families to be used in the CSS
            const formattedFontFamilies = [
                formatFamily(fontFamily),
                ...adjustFontFallbackFamily ? [
                    adjustFontFallbackFamily
                ] : [],
                ...fallbackFonts
            ].join(", ");
            // Add class with family, weight and style
            const classRule = new _postcss.default.Rule({
                selector: ".className"
            });
            classRule.nodes = [
                new _postcss.default.Declaration({
                    prop: "font-family",
                    value: formattedFontFamilies
                }),
                // If the font only has one weight or style, we can set it on the class
                ...weight && !isRange(weight) ? [
                    new _postcss.default.Declaration({
                        prop: "font-weight",
                        value: weight
                    })
                ] : [],
                ...style && !isRange(style) ? [
                    new _postcss.default.Declaration({
                        prop: "font-style",
                        value: style
                    })
                ] : []
            ];
            root.nodes.push(classRule);
            // Add CSS class that defines a variable with the font families
            if (variable) {
                const varialbeRule = new _postcss.default.Rule({
                    selector: ".variable"
                });
                varialbeRule.nodes = [
                    new _postcss.default.Declaration({
                        prop: variable,
                        value: formattedFontFamilies
                    })
                ];
                root.nodes.push(varialbeRule);
            }
            // Export @font-face values as is
            exports1.push({
                name: "style",
                value: {
                    fontFamily: formattedFontFamilies,
                    fontWeight: !Number.isNaN(Number(weight)) ? Number(weight) : undefined,
                    fontStyle: style && !isRange(style) ? style : undefined
                }
            });
        }
    };
};
postcssNextFontPlugin.postcss = true;
const _default = postcssNextFontPlugin;

//# sourceMappingURL=postcss-next-font.js.map