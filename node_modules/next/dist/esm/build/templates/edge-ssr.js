import '../../server/web/globals';
import { adapter } from '../../server/web/adapter';
import { getRender } from '../webpack/loaders/next-edge-ssr-loader/render';
import { IncrementalCache } from '../../server/lib/incremental-cache';
import Document from 'VAR_MODULE_DOCUMENT';
import * as appMod from 'VAR_MODULE_APP';
import * as userlandPage from 'VAR_USERLAND';
import * as userlandErrorPage from 'VAR_MODULE_GLOBAL_ERROR';
// OPTIONAL_IMPORT:* as userland500Page
// OPTIONAL_IMPORT:incrementalCacheHandler
// TODO: re-enable this once we've refactored to use implicit matches
// const renderToHTML = undefined
import { renderToHTML } from '../../server/render';
import RouteModule from '../../server/route-modules/pages/module';
// INJECT:pagesType
// INJECT:sriEnabled
// INJECT:dev
// INJECT:nextConfig
// INJECT:pageRouteModuleOptions
// INJECT:errorRouteModuleOptions
// INJECT:user500RouteModuleOptions
const cacheHandlers = {};
if (!globalThis.__nextCacheHandlers) {
    ;
    globalThis.__nextCacheHandlers = cacheHandlers;
}
const pageMod = {
    ...userlandPage,
    routeModule: new RouteModule({
        ...pageRouteModuleOptions,
        components: {
            App: appMod.default,
            Document
        },
        userland: userlandPage
    })
};
const errorMod = {
    ...userlandErrorPage,
    routeModule: new RouteModule({
        ...errorRouteModuleOptions,
        components: {
            App: appMod.default,
            Document
        },
        userland: userlandErrorPage
    })
};
// FIXME: this needs to be made compatible with the template
const error500Mod = userland500Page ? {
    ...userland500Page,
    routeModule: new RouteModule({
        ...user500RouteModuleOptions,
        components: {
            App: appMod.default,
            Document
        },
        userland: userland500Page
    })
} : null;
const maybeJSONParse = (str)=>str ? JSON.parse(str) : undefined;
const buildManifest = self.__BUILD_MANIFEST;
const reactLoadableManifest = maybeJSONParse(self.__REACT_LOADABLE_MANIFEST);
const dynamicCssManifest = maybeJSONParse(self.__DYNAMIC_CSS_MANIFEST);
const subresourceIntegrityManifest = sriEnabled ? maybeJSONParse(self.__SUBRESOURCE_INTEGRITY_MANIFEST) : undefined;
const nextFontManifest = maybeJSONParse(self.__NEXT_FONT_MANIFEST);
const render = getRender({
    pagesType,
    dev,
    page: 'VAR_PAGE',
    appMod,
    pageMod,
    errorMod,
    error500Mod,
    Document,
    buildManifest,
    renderToHTML,
    reactLoadableManifest,
    dynamicCssManifest,
    subresourceIntegrityManifest,
    config: nextConfig,
    buildId: process.env.__NEXT_BUILD_ID,
    nextFontManifest,
    incrementalCacheHandler
});
export const ComponentMod = pageMod;
export default function nHandler(opts) {
    return adapter({
        ...opts,
        IncrementalCache,
        handler: render
    });
}

//# sourceMappingURL=edge-ssr.js.map