// @ts-ignore
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "LightningCssMinifyPlugin", {
    enumerable: true,
    get: function() {
        return LightningCssMinifyPlugin;
    }
});
const _webpack = require("next/dist/compiled/webpack/webpack");
const _webpacksources3 = require("next/dist/compiled/webpack-sources3");
const _interface = require("./interface");
const _utils = require("./utils");
const _buffer = require("buffer");
const PLUGIN_NAME = "lightning-css-minify";
const CSS_FILE_REG = /\.css(?:\?.*)?$/i;
class LightningCssMinifyPlugin {
    constructor(opts = {}){
        const { implementation, ...otherOpts } = opts;
        if (implementation && typeof implementation.transformCss !== "function") {
            throw new TypeError(`[LightningCssMinifyPlugin]: implementation.transformCss must be an 'lightningcss' transform function. Received ${typeof implementation.transformCss}`);
        }
        this.transform = implementation == null ? void 0 : implementation.transformCss;
        this.options = otherOpts;
    }
    apply(compiler) {
        const meta = JSON.stringify({
            name: "@next/lightningcss-loader",
            version: "0.0.0",
            options: this.options
        });
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation)=>{
            compilation.hooks.chunkHash.tap(PLUGIN_NAME, (_, hash)=>hash.update(meta));
            compilation.hooks.processAssets.tapPromise({
                name: PLUGIN_NAME,
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_OPTIMIZE_SIZE,
                additionalAssets: true
            }, async ()=>await this.transformAssets(compilation));
            compilation.hooks.statsPrinter.tap(PLUGIN_NAME, (statsPrinter)=>{
                statsPrinter.hooks.print.for("asset.info.minimized")// @ts-ignore
                .tap(PLUGIN_NAME, (minimized, { green, formatFlag })=>{
                    // @ts-ignore
                    return minimized ? green(formatFlag("minimized")) : undefined;
                });
            });
        });
    }
    async transformAssets(compilation) {
        const { options: { devtool } } = compilation.compiler;
        if (!this.transform) {
            const { loadBindings } = require("next/dist/build/swc");
            this.transform = (await loadBindings()).css.lightning.transform;
        }
        const sourcemap = this.options.sourceMap === undefined ? devtool && devtool.includes("source-map") : this.options.sourceMap;
        const { include, exclude, test: testRegExp, targets: userTargets, ...transformOptions } = this.options;
        const assets = compilation.getAssets().filter((asset)=>// Filter out already minimized
            !asset.info.minimized && // Filter out by file type
            (testRegExp || CSS_FILE_REG).test(asset.name) && _webpack.ModuleFilenameHelpers.matchObject({
                include,
                exclude
            }, asset.name));
        await Promise.all(assets.map(async (asset)=>{
            const { source, map } = asset.source.sourceAndMap();
            const sourceAsString = source.toString();
            const code = typeof source === "string" ? _buffer.Buffer.from(source) : source;
            const targets = (0, _utils.getTargets)({
                targets: userTargets,
                key: _interface.ECacheKey.minify
            });
            const result = await this.transform({
                filename: asset.name,
                code,
                minify: true,
                sourceMap: sourcemap,
                targets,
                ...transformOptions
            });
            const codeString = result.code.toString();
            compilation.updateAsset(asset.name, // @ts-ignore
            sourcemap ? new _webpacksources3.SourceMapSource(codeString, asset.name, JSON.parse(result.map.toString()), sourceAsString, map, true) : new _webpacksources3.RawSource(codeString), {
                ...asset.info,
                minimized: true
            });
        }));
    }
}

//# sourceMappingURL=minify.js.map