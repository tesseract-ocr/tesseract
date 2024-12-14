"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getGlobalCssLoader", {
    enumerable: true,
    get: function() {
        return getGlobalCssLoader;
    }
});
const _client = require("./client");
const _fileresolve = require("./file-resolve");
function getGlobalCssLoader(ctx, postcss, preProcessors = []) {
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
                modules: false,
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
                // Next.js controls CSS Modules eligibility:
                modules: false,
                url: (url, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports),
                import: (url, _, resourcePath)=>(0, _fileresolve.cssFileResolve)(url, resourcePath, ctx.experimental.urlImports)
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

//# sourceMappingURL=global.js.map