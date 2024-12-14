import { base } from './blocks/base';
import { css } from './blocks/css';
import { images } from './blocks/images';
import { pipe } from './utils';
export async function buildConfiguration(config, { hasAppDir, supportedBrowsers, rootDirectory, customAppFile, isDevelopment, isServer, isEdgeRuntime, targetWeb, assetPrefix, sassOptions, productionBrowserSourceMaps, future, transpilePackages, experimental, disableStaticImages, serverSourceMaps }) {
    const ctx = {
        hasAppDir,
        supportedBrowsers,
        rootDirectory,
        customAppFile,
        isDevelopment,
        isProduction: !isDevelopment,
        isServer,
        isEdgeRuntime,
        isClient: !isServer,
        targetWeb,
        assetPrefix: assetPrefix ? assetPrefix.endsWith('/') ? assetPrefix.slice(0, -1) : assetPrefix : '',
        sassOptions,
        productionBrowserSourceMaps,
        transpilePackages,
        future,
        experimental,
        serverSourceMaps: serverSourceMaps ?? false
    };
    let fns = [
        base(ctx),
        css(ctx)
    ];
    if (!disableStaticImages) {
        fns.push(images(ctx));
    }
    const fn = pipe(...fns);
    return fn(config);
}

//# sourceMappingURL=index.js.map