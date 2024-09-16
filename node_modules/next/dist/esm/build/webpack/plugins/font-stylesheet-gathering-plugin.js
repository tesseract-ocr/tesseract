import { webpack, BasicEvaluatedExpression, sources } from "next/dist/compiled/webpack/webpack";
import { getFontDefinitionFromNetwork, getFontOverrideCss } from "../../../server/font-utils";
import postcss from "postcss";
import minifier from "next/dist/compiled/cssnano-simple";
import { AUTOMATIC_FONT_OPTIMIZATION_MANIFEST, OPTIMIZED_FONT_PROVIDERS } from "../../../shared/lib/constants";
import * as Log from "../../output/log";
function minifyCss(css) {
    return postcss([
        minifier({
            excludeAll: true,
            discardComments: true,
            normalizeWhitespace: {
                exclude: false
            }
        }, postcss)
    ]).process(css, {
        from: undefined
    }).then((res)=>res.css);
}
function isNodeCreatingLinkElement(node) {
    const callee = node.callee;
    if (callee.type !== "Identifier") {
        return false;
    }
    const componentNode = node.arguments[0];
    if (componentNode.type !== "Literal") {
        return false;
    }
    // React has pragma: _jsx.
    // Next has pragma: __jsx.
    return (callee.name === "_jsx" || callee.name === "__jsx") && componentNode.value === "link";
}
export class FontStylesheetGatheringPlugin {
    constructor({ adjustFontFallbacks, adjustFontFallbacksWithSizeAdjust }){
        this.gatheredStylesheets = [];
        this.manifestContent = [];
        this.parserHandler = (factory)=>{
            const JS_TYPES = [
                "auto",
                "esm",
                "dynamic"
            ];
            // Do an extra walk per module and add interested visitors to the walk.
            for (const type of JS_TYPES){
                factory.hooks.parser.for("javascript/" + type).tap(this.constructor.name, (parser)=>{
                    /**
           * Webpack fun facts:
           * `parser.hooks.call.for` cannot catch calls for user defined identifiers like `__jsx`
           * it can only detect calls for native objects like `window`, `this`, `eval` etc.
           * In order to be able to catch calls of variables like `__jsx`, first we need to catch them as
           * Identifier and then return `BasicEvaluatedExpression` whose `id` and `type` webpack matches to
           * invoke hook for call.
           * See: https://github.com/webpack/webpack/blob/webpack-4/lib/Parser.js#L1931-L1932.
           */ parser.hooks.evaluate.for("Identifier").tap(this.constructor.name, (node)=>{
                        var _parser_state_module, _parser_state;
                        // We will only optimize fonts from first party code.
                        if (parser == null ? void 0 : (_parser_state = parser.state) == null ? void 0 : (_parser_state_module = _parser_state.module) == null ? void 0 : _parser_state_module.resource.includes("node_modules")) {
                            return;
                        }
                        let result;
                        if (node.name === "_jsx" || node.name === "__jsx") {
                            result = new BasicEvaluatedExpression();
                            // @ts-ignore
                            result.setRange(node.range);
                            result.setExpression(node);
                            result.setIdentifier(node.name);
                            // This was added in webpack 5.
                            result.getMembers = ()=>[];
                        }
                        return result;
                    });
                    const jsxNodeHandler = (node)=>{
                        var _parser_state_module, _parser_state;
                        if (node.arguments.length !== 2) {
                            // A font link tag has only two arguments rel=stylesheet and href='...'
                            return;
                        }
                        if (!isNodeCreatingLinkElement(node)) {
                            return;
                        }
                        // node.arguments[0] is the name of the tag and [1] are the props.
                        const arg1 = node.arguments[1];
                        const propsNode = arg1.type === "ObjectExpression" ? arg1 : undefined;
                        const props = {};
                        if (propsNode) {
                            propsNode.properties.forEach((prop)=>{
                                if (prop.type !== "Property") {
                                    return;
                                }
                                if (prop.key.type === "Identifier" && prop.value.type === "Literal") {
                                    props[prop.key.name] = prop.value.value;
                                }
                            });
                        }
                        if (!props.rel || props.rel !== "stylesheet" || !props.href || !OPTIMIZED_FONT_PROVIDERS.some(({ url })=>props.href.startsWith(url))) {
                            return false;
                        }
                        this.gatheredStylesheets.push(props.href);
                        const buildInfo = parser == null ? void 0 : (_parser_state = parser.state) == null ? void 0 : (_parser_state_module = _parser_state.module) == null ? void 0 : _parser_state_module.buildInfo;
                        if (buildInfo) {
                            buildInfo.valueDependencies.set(AUTOMATIC_FONT_OPTIMIZATION_MANIFEST, this.gatheredStylesheets);
                        }
                    };
                    // React JSX transform:
                    parser.hooks.call.for("_jsx").tap(this.constructor.name, jsxNodeHandler);
                    // Next.js JSX transform:
                    parser.hooks.call.for("__jsx").tap(this.constructor.name, jsxNodeHandler);
                    // New React JSX transform:
                    parser.hooks.call.for("imported var").tap(this.constructor.name, jsxNodeHandler);
                });
            }
        };
        this.adjustFontFallbacks = adjustFontFallbacks;
        this.adjustFontFallbacksWithSizeAdjust = adjustFontFallbacksWithSizeAdjust;
    }
    apply(compiler) {
        this.compiler = compiler;
        compiler.hooks.normalModuleFactory.tap(this.constructor.name, this.parserHandler);
        compiler.hooks.make.tapAsync(this.constructor.name, (compilation, cb)=>{
            compilation.hooks.finishModules.tapAsync(this.constructor.name, async (modules, modulesFinished)=>{
                let fontStylesheets = this.gatheredStylesheets;
                const fontUrls = new Set();
                modules.forEach((module)=>{
                    var _module_buildInfo_valueDependencies, _module_buildInfo;
                    const fontDependencies = module == null ? void 0 : (_module_buildInfo = module.buildInfo) == null ? void 0 : (_module_buildInfo_valueDependencies = _module_buildInfo.valueDependencies) == null ? void 0 : _module_buildInfo_valueDependencies.get(AUTOMATIC_FONT_OPTIMIZATION_MANIFEST);
                    if (fontDependencies) {
                        fontDependencies.forEach((v)=>fontUrls.add(v));
                    }
                });
                fontStylesheets = Array.from(fontUrls);
                const fontDefinitionPromises = fontStylesheets.map((url)=>getFontDefinitionFromNetwork(url));
                this.manifestContent = [];
                for(let promiseIndex in fontDefinitionPromises){
                    let css = await fontDefinitionPromises[promiseIndex];
                    if (this.adjustFontFallbacks) {
                        css += getFontOverrideCss(fontStylesheets[promiseIndex], css, this.adjustFontFallbacksWithSizeAdjust);
                    }
                    if (css) {
                        try {
                            const content = await minifyCss(css);
                            this.manifestContent.push({
                                url: fontStylesheets[promiseIndex],
                                content
                            });
                        } catch (err) {
                            Log.warn(`Failed to minify the stylesheet for ${fontStylesheets[promiseIndex]}. Skipped optimizing this font.`);
                            console.error(err);
                        }
                    }
                }
                // @ts-expect-error invalid assets type
                compilation.assets[AUTOMATIC_FONT_OPTIMIZATION_MANIFEST] = new sources.RawSource(JSON.stringify(this.manifestContent, null, "  "));
                modulesFinished();
            });
            cb();
        });
        compiler.hooks.make.tap(this.constructor.name, (compilation)=>{
            compilation.hooks.processAssets.tap({
                name: this.constructor.name,
                stage: webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>{
                assets["../" + AUTOMATIC_FONT_OPTIMIZATION_MANIFEST] = new sources.RawSource(JSON.stringify(this.manifestContent, null, "  "));
            });
        });
    }
}

//# sourceMappingURL=font-stylesheet-gathering-plugin.js.map