"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "postProcessHTML", {
    enumerable: true,
    get: function() {
        return postProcessHTML;
    }
});
const _constants = require("../shared/lib/constants");
const _nonnullable = require("../lib/non-nullable");
const middlewareRegistry = [];
function registerPostProcessor(name, middleware, condition) {
    middlewareRegistry.push({
        name,
        middleware,
        condition: condition || null
    });
}
async function processHTML(html, data, options) {
    // Don't parse unless there's at least one processor middleware
    if (!middlewareRegistry[0]) {
        return html;
    }
    const { parse } = require("next/dist/compiled/node-html-parser");
    const root = parse(html);
    let document = html;
    // Calls the middleware, with some instrumentation and logging
    async function callMiddleWare(middleware) {
        // let timer = Date.now()
        const inspectData = middleware.inspect(root, data);
        document = await middleware.mutate(document, inspectData, data);
        // timer = Date.now() - timer
        // if (timer > MIDDLEWARE_TIME_BUDGET) {
        // TODO: Identify a correct upper limit for the postprocess step
        // and add a warning to disable the optimization
        // }
        return;
    }
    for(let i = 0; i < middlewareRegistry.length; i++){
        let middleware = middlewareRegistry[i];
        if (!middleware.condition || middleware.condition(options)) {
            await callMiddleWare(middlewareRegistry[i].middleware);
        }
    }
    return document;
}
class FontOptimizerMiddleware {
    inspect(originalDom, options) {
        if (!options.getFontDefinition) {
            return;
        }
        const fontDefinitions = [];
        // collecting all the requested font definitions
        originalDom.querySelectorAll("link").filter((tag)=>tag.getAttribute("rel") === "stylesheet" && tag.hasAttribute("data-href") && _constants.OPTIMIZED_FONT_PROVIDERS.some(({ url })=>{
                const dataHref = tag.getAttribute("data-href");
                return dataHref ? dataHref.startsWith(url) : false;
            })).forEach((element)=>{
            const url = element.getAttribute("data-href");
            const nonce = element.getAttribute("nonce");
            if (url) {
                fontDefinitions.push([
                    url,
                    nonce
                ]);
            }
        });
        return fontDefinitions;
    }
    constructor(){
        this.mutate = async (markup, fontDefinitions, options)=>{
            let result = markup;
            let preconnectUrls = new Set();
            if (!options.getFontDefinition) {
                return markup;
            }
            fontDefinitions.forEach((fontDef)=>{
                const [url, nonce] = fontDef;
                const fallBackLinkTag = `<link rel="stylesheet" href="${url}"/>`;
                if (result.indexOf(`<style data-href="${url}">`) > -1 || result.indexOf(fallBackLinkTag) > -1) {
                    // The font is already optimized and probably the response is cached
                    return;
                }
                const fontContent = options.getFontDefinition ? options.getFontDefinition(url) : null;
                if (!fontContent) {
                    /**
         * In case of unreachable font definitions, fallback to default link tag.
         */ result = result.replace("</head>", `${fallBackLinkTag}</head>`);
                } else {
                    const nonceStr = nonce ? ` nonce="${nonce}"` : "";
                    let dataAttr = "";
                    if (fontContent.includes("ascent-override")) {
                        dataAttr = ' data-size-adjust="true"';
                    }
                    result = result.replace("</head>", `<style data-href="${url}"${nonceStr}${dataAttr}>${fontContent}</style></head>`);
                    // Remove inert font tag
                    const escapedUrl = url.replace(/&/g, "&amp;").replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
                    const fontRegex = new RegExp(`<link[^>]*data-href="${escapedUrl}"[^>]*/>`);
                    result = result.replace(fontRegex, "");
                    const provider = _constants.OPTIMIZED_FONT_PROVIDERS.find((p)=>url.startsWith(p.url));
                    if (provider) {
                        preconnectUrls.add(provider.preconnect);
                    }
                }
            });
            let preconnectTag = "";
            preconnectUrls.forEach((url)=>{
                preconnectTag += `<link rel="preconnect" href="${url}" crossorigin />`;
            });
            result = result.replace('<meta name="next-font-preconnect"/>', preconnectTag);
            return result;
        };
    }
}
async function postProcessHTML(pathname, content, renderOpts, { inAmpMode, hybridAmp }) {
    const postProcessors = [
        process.env.NEXT_RUNTIME !== "edge" && inAmpMode ? async (html)=>{
            const optimizeAmp = require("./optimize-amp").default;
            html = await optimizeAmp(html, renderOpts.ampOptimizerConfig);
            if (!renderOpts.ampSkipValidation && renderOpts.ampValidator) {
                await renderOpts.ampValidator(html, pathname);
            }
            return html;
        } : null,
        process.env.NEXT_RUNTIME !== "edge" && renderOpts.optimizeFonts ? async (html)=>{
            const getFontDefinition = (url)=>{
                var _renderOpts_fontManifest_find;
                if (!renderOpts.fontManifest) {
                    return "";
                }
                return ((_renderOpts_fontManifest_find = renderOpts.fontManifest.find((font)=>{
                    if (font && font.url === url) {
                        return true;
                    }
                    return false;
                })) == null ? void 0 : _renderOpts_fontManifest_find.content) || "";
            };
            return await processHTML(html, {
                getFontDefinition
            }, {
                optimizeFonts: renderOpts.optimizeFonts
            });
        } : null,
        process.env.NEXT_RUNTIME !== "edge" && renderOpts.optimizeCss ? async (html)=>{
            // eslint-disable-next-line import/no-extraneous-dependencies
            const Critters = require("critters");
            const cssOptimizer = new Critters({
                ssrMode: true,
                reduceInlineStyles: false,
                path: renderOpts.distDir,
                publicPath: `${renderOpts.assetPrefix}/_next/`,
                preload: "media",
                fonts: false,
                ...renderOpts.optimizeCss
            });
            return await cssOptimizer.process(html);
        } : null,
        inAmpMode || hybridAmp ? (html)=>{
            return html.replace(/&amp;amp=1/g, "&amp=1");
        } : null
    ].filter(_nonnullable.nonNullable);
    for (const postProcessor of postProcessors){
        if (postProcessor) {
            content = await postProcessor(content);
        }
    }
    return content;
}
// Initialization
registerPostProcessor("Inline-Fonts", new FontOptimizerMiddleware(), // Using process.env because passing Experimental flag through loader is not possible.
// @ts-ignore
(options)=>options.optimizeFonts || process.env.__NEXT_OPTIMIZE_FONTS);

//# sourceMappingURL=post-process.js.map