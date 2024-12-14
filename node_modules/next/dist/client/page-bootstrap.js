"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "pageBootstrap", {
    enumerable: true,
    get: function() {
        return pageBootstrap;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _ = require("./");
const _ondemandentriesclient = /*#__PURE__*/ _interop_require_default._(require("./dev/on-demand-entries-client"));
const _devbuildwatcher = /*#__PURE__*/ _interop_require_default._(require("./dev/dev-build-watcher"));
const _fouc = require("./dev/fouc");
const _websocket = require("./components/react-dev-overlay/pages/websocket");
const _querystring = require("../shared/lib/router/utils/querystring");
const _hotreloadertypes = require("../server/dev/hot-reloader-types");
const _runtimeerrorhandler = require("./components/react-dev-overlay/internal/helpers/runtime-error-handler");
const _shared = require("./components/react-dev-overlay/shared");
const _hotreloaderclient = require("./components/react-dev-overlay/pages/hot-reloader-client");
function pageBootstrap(assetPrefix) {
    (0, _websocket.connectHMR)({
        assetPrefix,
        path: '/_next/webpack-hmr'
    });
    return (0, _.hydrate)({
        beforeRender: _fouc.displayContent
    }).then(()=>{
        (0, _ondemandentriesclient.default)();
        let buildIndicatorHandler;
        if (process.env.__NEXT_BUILD_INDICATOR) {
            (0, _devbuildwatcher.default)((handler)=>{
                buildIndicatorHandler = handler;
            }, process.env.__NEXT_BUILD_INDICATOR_POSITION);
        }
        let reloading = false;
        (0, _websocket.addMessageListener)((payload)=>{
            if (reloading) return;
            if ('action' in payload) {
                switch(payload.action){
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_ERROR:
                        {
                            const { stack, message } = JSON.parse(payload.errorJSON);
                            const error = new Error(message);
                            error.stack = stack;
                            throw error;
                        }
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.RELOAD_PAGE:
                        {
                            reloading = true;
                            window.location.reload();
                            break;
                        }
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.DEV_PAGES_MANIFEST_UPDATE:
                        {
                            fetch("" + assetPrefix + "/_next/static/development/_devPagesManifest.json").then((res)=>res.json()).then((manifest)=>{
                                window.__DEV_PAGES_MANIFEST = manifest;
                            }).catch((err)=>{
                                console.log("Failed to fetch devPagesManifest", err);
                            });
                            break;
                        }
                    default:
                        break;
                }
            } else if ('event' in payload) {
                switch(payload.event){
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.MIDDLEWARE_CHANGES:
                        {
                            return window.location.reload();
                        }
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.CLIENT_CHANGES:
                        {
                            // This is used in `../server/dev/turbopack-utils.ts`.
                            const isOnErrorPage = window.next.router.pathname === '/_error';
                            // On the error page we want to reload the page when a page was changed
                            if (isOnErrorPage) {
                                if (_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError) {
                                    console.warn(_shared.REACT_REFRESH_FULL_RELOAD_FROM_ERROR);
                                }
                                reloading = true;
                                (0, _hotreloaderclient.performFullReload)(null);
                            }
                            break;
                        }
                    case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_ONLY_CHANGES:
                        {
                            if (_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError) {
                                console.warn(_shared.REACT_REFRESH_FULL_RELOAD_FROM_ERROR);
                                (0, _hotreloaderclient.performFullReload)(null);
                            }
                            const { pages } = payload;
                            // Make sure to reload when the dev-overlay is showing for an
                            // API route
                            // TODO: Fix `__NEXT_PAGE` type
                            if (pages.includes(_.router.query.__NEXT_PAGE)) {
                                return window.location.reload();
                            }
                            if (!_.router.clc && pages.includes(_.router.pathname)) {
                                console.log('Refreshing page data due to server-side change');
                                buildIndicatorHandler == null ? void 0 : buildIndicatorHandler.show();
                                const clearIndicator = ()=>buildIndicatorHandler == null ? void 0 : buildIndicatorHandler.hide();
                                _.router.replace(_.router.pathname + '?' + String((0, _querystring.assign)((0, _querystring.urlQueryToSearchParams)(_.router.query), new URLSearchParams(location.search))), _.router.asPath, {
                                    scroll: false
                                }).catch(()=>{
                                    // trigger hard reload when failing to refresh data
                                    // to show error overlay properly
                                    location.reload();
                                }).finally(clearIndicator);
                            }
                            break;
                        }
                    default:
                        break;
                }
            }
        });
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=page-bootstrap.js.map