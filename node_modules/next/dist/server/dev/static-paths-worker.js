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
const _appsegments = require("../../build/segment-config/app/app-segments");
const _loadcomponents = require("../load-components");
const _setuphttpagentenv = require("../setup-http-agent-env");
const _checks = require("../route-modules/checks");
const _ppr = require("../lib/experimental/ppr");
const _invarianterror = require("../../shared/lib/invariant-error");
async function loadStaticPaths({ dir, distDir, pathname, config, httpAgentOptions, locales, defaultLocale, isAppPath, page, isrFlushToDisk, fetchCacheKeyPrefix, maxMemoryCacheSize, requestHeaders, cacheHandler, cacheLifeProfiles, nextConfigOutput, buildId, authInterrupts }) {
    // update work memory runtime-config
    require('../../shared/lib/runtime-config.external').setConfig(config);
    (0, _setuphttpagentenv.setHttpClientAndAgentOptions)({
        httpAgentOptions
    });
    const components = await (0, _loadcomponents.loadComponents)({
        distDir,
        // In `pages/`, the page is the same as the pathname.
        page: page || pathname,
        isAppPath
    });
    if (isAppPath) {
        const segments = await (0, _appsegments.collectSegments)(components);
        const isRoutePPREnabled = (0, _checks.isAppPageRouteModule)(components.routeModule) && (0, _ppr.checkIsRoutePPREnabled)(config.pprConfig, (0, _utils.reduceAppConfig)(segments));
        return (0, _utils.buildAppStaticPaths)({
            dir,
            page: pathname,
            dynamicIO: config.dynamicIO,
            segments,
            configFileName: config.configFileName,
            distDir,
            requestHeaders,
            cacheHandler,
            cacheLifeProfiles,
            isrFlushToDisk,
            fetchCacheKeyPrefix,
            maxMemoryCacheSize,
            ComponentMod: components.ComponentMod,
            nextConfigOutput,
            isRoutePPREnabled,
            buildId,
            authInterrupts
        });
    } else if (!components.getStaticPaths) {
        // We shouldn't get to this point since the worker should only be called for
        // SSG pages with getStaticPaths.
        throw new _invarianterror.InvariantError(`Failed to load page with getStaticPaths for ${pathname}`);
    }
    return (0, _utils.buildStaticPaths)({
        page: pathname,
        getStaticPaths: components.getStaticPaths,
        configFileName: config.configFileName,
        locales,
        defaultLocale
    });
}

//# sourceMappingURL=static-paths-worker.js.map