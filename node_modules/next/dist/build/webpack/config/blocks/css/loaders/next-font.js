"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getNextFontLoader", {
    enumerable: true,
    get: function() {
        return getNextFontLoader;
    }
});
const _client = require("./client");
const _fileresolve = require("./file-resolve");
function getNextFontLoader(ctx, postcss, fontLoaderPath) {
    const loaders = [];
    if (ctx.isClient) {
        // Add appropriate development mode or production mode style
        // loader
        loaders.push((0, _client.getClientStyleLoader)({
            hasAppDir: ctx.hasAppDir,
            isDevelopment: ctx.isDevelopment,
            assetPrefix: ctx.assetPrefix
        }));
    }
    loaders.push({
        loader: require.resolve('../../../../loaders/css-loader/src'),
        options: {
            postcss,
            importLoaders: 1,
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
                getLocalIdent: (_context, _localIdentName, exportName, _options, meta)=>{
                    // hash from next-font-loader
                    return `__${exportName}_${meta.fontFamilyHash}`;
                }
            },
            fontLoader: true
        }
    });
    loaders.push({
        loader: 'next-font-loader',
        options: {
            isDev: ctx.isDevelopment,
            isServer: ctx.isServer,
            assetPrefix: ctx.assetPrefix,
            fontLoaderPath,
            postcss
        }
    });
    return loaders;
}

//# sourceMappingURL=next-font.js.map