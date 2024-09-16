"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "loadStaticPaths", {
    enumerable: true,
    get: function() {
        return loadStaticPaths;
    }
});
require("../require-hook");
require("../node-environment");
const _utils = require("../../build/utils");
const _loadcomponents = require("../load-components");
const _setuphttpagentenv = require("../setup-http-agent-env");
const _checks = require("../future/route-modules/checks");
async function loadStaticPaths({ dir, distDir, pathname, config, httpAgentOptions, locales, defaultLocale, isAppPath, page, isrFlushToDisk, fetchCacheKeyPrefix, maxMemoryCacheSize, requestHeaders, cacheHandler, ppr }) {
    // update work memory runtime-config
    require("../../shared/lib/runtime-config.external").setConfig(config);
    (0, _setuphttpagentenv.setHttpClientAndAgentOptions)({
        httpAgentOptions
    });
    const components = await (0, _loadcomponents.loadComponents)({
        distDir,
        // In `pages/`, the page is the same as the pathname.
        page: page || pathname,
        isAppPath
    });
    if (!components.getStaticPaths && !isAppPath) {
        // we shouldn't get to this point since the worker should
        // only be called for SSG pages with getStaticPaths
        throw new Error(`Invariant: failed to load page with getStaticPaths for ${pathname}`);
    }
    if (isAppPath) {
        const { routeModule } = components;
        const generateParams = routeModule && (0, _checks.isAppRouteRouteModule)(routeModule) ? [
            {
                config: {
                    revalidate: routeModule.userland.revalidate,
                    dynamic: routeModule.userland.dynamic,
                    dynamicParams: routeModule.userland.dynamicParams
                },
                generateStaticParams: routeModule.userland.generateStaticParams,
                segmentPath: pathname
            }
        ] : await (0, _utils.collectGenerateParams)(components.ComponentMod.tree);
        return await (0, _utils.buildAppStaticPaths)({
            dir,
            page: pathname,
            generateParams,
            configFileName: config.configFileName,
            distDir,
            requestHeaders,
            cacheHandler,
            isrFlushToDisk,
            fetchCacheKeyPrefix,
            maxMemoryCacheSize,
            ppr,
            ComponentMod: components.ComponentMod
        });
    }
    return await (0, _utils.buildStaticPaths)({
        page: pathname,
        getStaticPaths: components.getStaticPaths,
        configFileName: config.configFileName,
        locales,
        defaultLocale
    });
}

//# sourceMappingURL=static-paths-worker.js.map