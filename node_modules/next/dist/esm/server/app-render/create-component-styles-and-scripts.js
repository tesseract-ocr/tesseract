import { jsx as _jsx } from "react/jsx-runtime";
import React from 'react';
import { interopDefault } from './interop-default';
import { getLinkAndScriptTags } from './get-css-inlined-link-tags';
import { getAssetQueryString } from './get-asset-query-string';
import { encodeURIPath } from '../../shared/lib/encode-uri-path';
import { renderCssResource } from './render-css-resource';
export async function createComponentStylesAndScripts({ filePath, getComponent, injectedCSS, injectedJS, ctx }) {
    const { styles: entryCssFiles, scripts: jsHrefs } = getLinkAndScriptTags(ctx.clientReferenceManifest, filePath, injectedCSS, injectedJS);
    const styles = renderCssResource(entryCssFiles, ctx);
    const scripts = jsHrefs ? jsHrefs.map((href, index)=>/*#__PURE__*/ _jsx("script", {
            src: `${ctx.assetPrefix}/_next/${encodeURIPath(href)}${getAssetQueryString(ctx, true)}`,
            async: true
        }, `script-${index}`)) : null;
    const Comp = interopDefault(await getComponent());
    return [
        Comp,
        styles,
        scripts
    ];
}

//# sourceMappingURL=create-component-styles-and-scripts.js.map