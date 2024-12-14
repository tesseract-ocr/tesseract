import '../require-hook';
import '../node-environment';
import { buildAppStaticPaths, buildStaticPaths, reduceAppConfig } from '../../build/utils';
import { collectSegments } from '../../build/segment-config/app/app-segments';
import { loadComponents } from '../load-components';
import { setHttpClientAndAgentOptions } from '../setup-http-agent-env';
import { isAppPageRouteModule } from '../route-modules/checks';
import { checkIsRoutePPREnabled } from '../lib/experimental/ppr';
import { InvariantError } from '../../shared/lib/invariant-error';
// we call getStaticPaths in a separate process to ensure
// side-effects aren't relied on in dev that will break
// during a production build
export async function loadStaticPaths({ dir, distDir, pathname, config, httpAgentOptions, locales, defaultLocale, isAppPath, page, isrFlushToDisk, fetchCacheKeyPrefix, maxMemoryCacheSize, requestHeaders, cacheHandler, cacheLifeProfiles, nextConfigOutput, buildId, authInterrupts }) {
    // update work memory runtime-config
    require('../../shared/lib/runtime-config.external').setConfig(config);
    setHttpClientAndAgentOptions({
        httpAgentOptions
    });
    const components = await loadComponents({
        distDir,
        // In `pages/`, the page is the same as the pathname.
        page: page || pathname,
        isAppPath
    });
    if (isAppPath) {
        const segments = await collectSegments(components);
        const isRoutePPREnabled = isAppPageRouteModule(components.routeModule) && checkIsRoutePPREnabled(config.pprConfig, reduceAppConfig(segments));
        return buildAppStaticPaths({
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
        throw new InvariantError(`Failed to load page with getStaticPaths for ${pathname}`);
    }
    return buildStaticPaths({
        page: pathname,
        getStaticPaths: components.getStaticPaths,
        configFileName: config.configFileName,
        locales,
        defaultLocale
    });
}

//# sourceMappingURL=static-paths-worker.js.map