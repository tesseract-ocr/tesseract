import { getClientStyleLoader } from "./client";
import { cssFileResolve } from "./file-resolve";
export function getNextFontLoader(ctx, postcss, fontLoaderPath) {
    const loaders = [];
    if (ctx.isClient) {
        // Add appropriate development mode or production mode style
        // loader
        loaders.push(getClientStyleLoader({
            hasAppDir: ctx.hasAppDir,
            isDevelopment: ctx.isDevelopment,
            assetPrefix: ctx.assetPrefix
        }));
    }
    loaders.push({
        loader: require.resolve("../../../../loaders/css-loader/src"),
        options: {
            postcss,
            importLoaders: 1,
            // Use CJS mode for backwards compatibility:
            esModule: false,
            url: (url, resourcePath)=>cssFileResolve(url, resourcePath, ctx.experimental.urlImports),
            import: (url, _, resourcePath)=>cssFileResolve(url, resourcePath, ctx.experimental.urlImports),
            modules: {
                // Do not transform class names (CJS mode backwards compatibility):
                exportLocalsConvention: "asIs",
                // Server-side (Node.js) rendering support:
                exportOnlyLocals: ctx.isServer,
                // Disallow global style exports so we can code-split CSS and
                // not worry about loading order.
                mode: "pure",
                getLocalIdent: (_context, _localIdentName, exportName, _options, meta)=>{
                    // hash from next-font-loader
                    return `__${exportName}_${meta.fontFamilyHash}`;
                }
            },
            fontLoader: true
        }
    });
    loaders.push({
        loader: "next-font-loader",
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