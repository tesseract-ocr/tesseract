"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getRender", {
    enumerable: true,
    get: function() {
        return getRender;
    }
});
const _webserver = /*#__PURE__*/ _interop_require_default(require("../../../../server/web-server"));
const _web = require("../../../../server/base-http/web");
const _constants = require("../../../../lib/constants");
const _apppaths = require("../../../../shared/lib/router/utils/app-paths");
const _internaledgewaituntil = require("../../../../server/web/internal-edge-wait-until");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getRender({ dev, page, appMod, pageMod, errorMod, error500Mod, pagesType, Document, buildManifest, reactLoadableManifest, dynamicCssManifest, interceptionRouteRewrites, renderToHTML, clientReferenceManifest, subresourceIntegrityManifest, serverActionsManifest, serverActions, config, buildId, nextFontManifest, incrementalCacheHandler }) {
    const isAppPath = pagesType === 'app';
    const baseLoadComponentResult = {
        dev,
        buildManifest,
        reactLoadableManifest,
        dynamicCssManifest,
        subresourceIntegrityManifest,
        Document,
        App: appMod == null ? void 0 : appMod.default,
        clientReferenceManifest
    };
    const server = new _webserver.default({
        dev,
        conf: config,
        minimalMode: true,
        webServerConfig: {
            page,
            pathname: isAppPath ? (0, _apppaths.normalizeAppPath)(page) : page,
            pagesType,
            interceptionRouteRewrites,
            extendRenderOpts: {
                buildId,
                runtime: _constants.SERVER_RUNTIME.experimentalEdge,
                supportsDynamicResponse: true,
                disableOptimizedLoading: true,
                serverActionsManifest,
                serverActions,
                nextFontManifest
            },
            renderToHTML,
            incrementalCacheHandler,
            loadComponent: async (inputPage)=>{
                if (inputPage === page) {
                    return {
                        ...baseLoadComponentResult,
                        Component: pageMod.default,
                        pageConfig: pageMod.config || {},
                        getStaticProps: pageMod.getStaticProps,
                        getServerSideProps: pageMod.getServerSideProps,
                        getStaticPaths: pageMod.getStaticPaths,
                        ComponentMod: pageMod,
                        isAppPath: !!pageMod.__next_app__,
                        page: inputPage,
                        routeModule: pageMod.routeModule
                    };
                }
                // If there is a custom 500 page, we need to handle it separately.
                if (inputPage === '/500' && error500Mod) {
                    return {
                        ...baseLoadComponentResult,
                        Component: error500Mod.default,
                        pageConfig: error500Mod.config || {},
                        getStaticProps: error500Mod.getStaticProps,
                        getServerSideProps: error500Mod.getServerSideProps,
                        getStaticPaths: error500Mod.getStaticPaths,
                        ComponentMod: error500Mod,
                        page: inputPage,
                        routeModule: error500Mod.routeModule
                    };
                }
                if (inputPage === '/_error') {
                    return {
                        ...baseLoadComponentResult,
                        Component: errorMod.default,
                        pageConfig: errorMod.config || {},
                        getStaticProps: errorMod.getStaticProps,
                        getServerSideProps: errorMod.getServerSideProps,
                        getStaticPaths: errorMod.getStaticPaths,
                        ComponentMod: errorMod,
                        page: inputPage,
                        routeModule: errorMod.routeModule
                    };
                }
                return null;
            }
        }
    });
    const handler = server.getRequestHandler();
    return async function render(request, event) {
        const extendedReq = new _web.WebNextRequest(request);
        const extendedRes = new _web.WebNextResponse(undefined);
        handler(extendedReq, extendedRes);
        const result = await extendedRes.toResponse();
        request.fetchMetrics = extendedReq.fetchMetrics;
        if (event == null ? void 0 : event.waitUntil) {
            // TODO(after):
            // remove `internal_runWithWaitUntil` and the `internal-edge-wait-until` module
            // when consumers switch to `after`.
            const waitUntilPromise = (0, _internaledgewaituntil.internal_getCurrentFunctionWaitUntil)();
            if (waitUntilPromise) {
                event.waitUntil(waitUntilPromise);
            }
        }
        return result;
    };
}

//# sourceMappingURL=render.js.map