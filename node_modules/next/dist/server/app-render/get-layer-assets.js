"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getLayerAssets", {
    enumerable: true,
    get: function() {
        return getLayerAssets;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _getcssinlinedlinktags = require("./get-css-inlined-link-tags");
const _getpreloadablefonts = require("./get-preloadable-fonts");
const _getassetquerystring = require("./get-asset-query-string");
const _encodeuripath = require("../../shared/lib/encode-uri-path");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getLayerAssets({ ctx, layoutOrPagePath, injectedCSS: injectedCSSWithCurrentLayout, injectedJS: injectedJSWithCurrentLayout, injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout }) {
    const { styles: styleTags, scripts: scriptTags } = layoutOrPagePath ? (0, _getcssinlinedlinktags.getLinkAndScriptTags)(ctx.clientReferenceManifest, layoutOrPagePath, injectedCSSWithCurrentLayout, injectedJSWithCurrentLayout, true) : {
        styles: [],
        scripts: []
    };
    const preloadedFontFiles = layoutOrPagePath ? (0, _getpreloadablefonts.getPreloadableFonts)(ctx.renderOpts.nextFontManifest, layoutOrPagePath, injectedFontPreloadTagsWithCurrentLayout) : null;
    if (preloadedFontFiles) {
        if (preloadedFontFiles.length) {
            for(let i = 0; i < preloadedFontFiles.length; i++){
                const fontFilename = preloadedFontFiles[i];
                const ext = /\.(woff|woff2|eot|ttf|otf)$/.exec(fontFilename)[1];
                const type = `font/${ext}`;
                const href = `${ctx.assetPrefix}/_next/${(0, _encodeuripath.encodeURIPath)(fontFilename)}`;
                ctx.componentMod.preloadFont(href, type, ctx.renderOpts.crossOrigin);
            }
        } else {
            try {
                let url = new URL(ctx.assetPrefix);
                ctx.componentMod.preconnect(url.origin, "anonymous");
            } catch (error) {
                // assetPrefix must not be a fully qualified domain name. We assume
                // we should preconnect to same origin instead
                ctx.componentMod.preconnect("/", "anonymous");
            }
        }
    }
    const styles = styleTags ? styleTags.map((href, index)=>{
        // In dev, Safari and Firefox will cache the resource during HMR:
        // - https://github.com/vercel/next.js/issues/5860
        // - https://bugs.webkit.org/show_bug.cgi?id=187726
        // Because of this, we add a `?v=` query to bypass the cache during
        // development. We need to also make sure that the number is always
        // increasing.
        const fullHref = `${ctx.assetPrefix}/_next/${(0, _encodeuripath.encodeURIPath)(href)}${(0, _getassetquerystring.getAssetQueryString)(ctx, true)}`;
        // `Precedence` is an opt-in signal for React to handle resource
        // loading and deduplication, etc. It's also used as the key to sort
        // resources so they will be injected in the correct order.
        // During HMR, it's critical to use different `precedence` values
        // for different stylesheets, so their order will be kept.
        // https://github.com/facebook/react/pull/25060
        const precedence = process.env.NODE_ENV === "development" ? "next_" + href : "next";
        ctx.componentMod.preloadStyle(fullHref, ctx.renderOpts.crossOrigin);
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
            rel: "stylesheet",
            href: fullHref,
            // @ts-ignore
            precedence: precedence,
            crossOrigin: ctx.renderOpts.crossOrigin
        }, index);
    }) : [];
    const scripts = scriptTags ? scriptTags.map((href, index)=>{
        const fullSrc = `${ctx.assetPrefix}/_next/${(0, _encodeuripath.encodeURIPath)(href)}${(0, _getassetquerystring.getAssetQueryString)(ctx, true)}`;
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("script", {
            src: fullSrc,
            async: true
        }, `script-${index}`);
    }) : [];
    return styles.length || scripts.length ? [
        ...styles,
        ...scripts
    ] : null;
}

//# sourceMappingURL=get-layer-assets.js.map