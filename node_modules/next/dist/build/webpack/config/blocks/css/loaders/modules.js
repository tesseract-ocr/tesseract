"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCssModuleLoader", {
    enumerable: true,
    get: function() {
        return getCssModuleLoader;
    }
});
const _client = require("./client");
const _fileresolve = require("./file-resolve");
const _getCssModuleLocalIdent = require("./getCssModuleLocalIdent");
function getCssModuleLoader(ctx, postcss, preProcessors = []) {
    const loaders = [];
    if (ctx.isClient) {
        // Add appropriate development more or production mode style
        // loader
        loaders.push((0, _client.getClientStyleLoader)({
            hasAppDir: ctx.hasAppDir,
            isAppDir: ctx.isAppDir,
            isDevelopment: ctx.isDevelopment,
            assetPrefix: ctx.assetPrefix
        }));
    }
    if (ctx.experimental.useLightningcss) {
        loaders.push({
            loader: require.resolve('../../../../loaders/lightningcss-loader/src'),
            options: {
                importLoaders: 1 + preProcessors.length,
                url: (url, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports),
                import: (url, _, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports),
                modules: {
                    // Do not transform class names (CJS mode backwards compatibility):
                    exportLocalsConvention: 'asIs',
                    // Server-side (Node.js) rendering support:
                    exportOnlyLocals: ctx.isServer
                },
                targets: ctx.supportedBrowsers,
                postcss
            }
        });
    } else {
        // Resolve CSS `@import`s and `url()`s
        loaders.push({
            loader: require.resolve('../../../../loaders/css-loader/src'),
            options: {
                postcss,
                importLoaders: 1 + preProcessors.length,
                // Use CJS mode for backwards compatibility:
                esModule: false,
                url: (url, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports),
                import: (url, _, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports),
                modules: {
                    // Do not transform class names (CJS mode backwards compatibility):
                    exportLocalsConvention: 'asIs',
                    // Server-side (Node.js) rendering support:
                    exportOnlyLocals: ctx.isServer,
                    // Disallow global style exports so we can code-split CSS and
                    // not worry about loading order.
                    mode: 'pure',
                    // Generate a friendly production-ready name so it's
                    // reasonably understandable. The same name is used for
                    // development.
                    // TODO: Consider making production reduce this to a single
                    // character?
                    getLocalIdent: _getCssModuleLocalIdent.getCssModuleLocalIdent
                }
            }
        });
        // Compile CSS
        loaders.push({
            loader: require.resolve('../../../../loaders/postcss-loader/src'),
            options: {
                postcss
            }
        });
    }
    loaders.push(// Webpack loaders run like a stack, so we need to reverse the natural
    // order of preprocessors.
    ...preProcessors.slice().reverse());
    return loaders;
}

//# sourceMappingURL=modules.js.map