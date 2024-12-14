"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "buildConfiguration", {
    enumerable: true,
    get: function() {
        return buildConfiguration;
    }
});
const _base = require("./blocks/base");
const _css = require("./blocks/css");
const _images = require("./blocks/images");
const _utils = require("./utils");
async function buildConfiguration(config, { hasAppDir, supportedBrowsers, rootDirectory, customAppFile, isDevelopment, isServer, isEdgeRuntime, targetWeb, assetPrefix, sassOptions, productionBrowserSourceMaps, future, transpilePackages, experimental, disableStaticImages, serverSourceMaps }) {
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
        (0, _base.base)(ctx),
        (0, _css.css)(ctx)
    ];
    if (!disableStaticImages) {
        fns.push((0, _images.images)(ctx));
    }
    const fn = (0, _utils.pipe)(...fns);
    return fn(config);
}

//# sourceMappingURL=index.js.map