import { jsx as _jsx } from "react/jsx-runtime";
import React from 'react';
import { getLinkAndScriptTags } from './get-css-inlined-link-tags';
import { getPreloadableFonts } from './get-preloadable-fonts';
import { getAssetQueryString } from './get-asset-query-string';
import { encodeURIPath } from '../../shared/lib/encode-uri-path';
import { renderCssResource } from './render-css-resource';
export function getLayerAssets({ ctx, layoutOrPagePath, injectedCSS: injectedCSSWithCurrentLayout, injectedJS: injectedJSWithCurrentLayout, injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout, preloadCallbacks }) {
    const { styles: styleTags, scripts: scriptTags } = layoutOrPagePath ? getLinkAndScriptTags(ctx.clientReferenceManifest, layoutOrPagePath, injectedCSSWithCurrentLayout, injectedJSWithCurrentLayout, true) : {
        styles: [],
        scripts: []
    };
    const preloadedFontFiles = layoutOrPagePath ? getPreloadableFonts(ctx.renderOpts.nextFontManifest, layoutOrPagePath, injectedFontPreloadTagsWithCurrentLayout) : null;
    if (preloadedFontFiles) {
        if (preloadedFontFiles.length) {
            for(let i = 0; i < preloadedFontFiles.length; i++){
                const fontFilename = preloadedFontFiles[i];
                const ext = /\.(woff|woff2|eot|ttf|otf)$/.exec(fontFilename)[1];
                const type = `font/${ext}`;
                const href = `${ctx.assetPrefix}/_next/${encodeURIPath(fontFilename)}`;
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
    const styles = renderCssResource(styleTags, ctx, preloadCallbacks);
    const scripts = scriptTags ? scriptTags.map((href, index)=>{
        const fullSrc = `${ctx.assetPrefix}/_next/${encodeURIPath(href)}${getAssetQueryString(ctx, true)}`;
        return /*#__PURE__*/ _jsx("script", {
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