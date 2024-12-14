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
const _rendercssresource = require("./render-css-resource");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getLayerAssets({ ctx, layoutOrPagePath, injectedCSS: injectedCSSWithCurrentLayout, injectedJS: injectedJSWithCurrentLayout, injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout, preloadCallbacks }) {
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
                preloadCallbacks.push(()=>{
                    ctx.componentMod.preloadFont(href, type, ctx.renderOpts.crossOrigin, ctx.nonce);
                });
            }
        } else {
            try {
                let url = new URL(ctx.assetPrefix);
                preloadCallbacks.push(()=>{
                    ctx.componentMod.preconnect(url.origin, 'anonymous', ctx.nonce);
                });
            } catch (error) {
                // assetPrefix must not be a fully qualified domain name. We assume
                // we should preconnect to same origin instead
                preloadCallbacks.push(()=>{
                    ctx.componentMod.preconnect('/', 'anonymous', ctx.nonce);
                });
            }
        }
    }
    const styles = (0, _rendercssresource.renderCssResource)(styleTags, ctx, preloadCallbacks);
    const scripts = scriptTags ? scriptTags.map((href, index)=>{
        const fullSrc = `${ctx.assetPrefix}/_next/${(0, _encodeuripath.encodeURIPath)(href)}${(0, _getassetquerystring.getAssetQueryString)(ctx, true)}`;
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("script", {
            src: fullSrc,
            async: true,
            nonce: ctx.nonce
        }, `script-${index}`);
    }) : [];
    return styles.length || scripts.length ? [
        ...styles,
        ...scripts
    ] : null;
}

//# sourceMappingURL=get-layer-assets.js.map