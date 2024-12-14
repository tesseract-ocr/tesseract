"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _hotreloaderclient = /*#__PURE__*/ _interop_require_default._(require("../components/react-dev-overlay/pages/hot-reloader-client"));
const _websocket = require("../components/react-dev-overlay/pages/websocket");
let reloading = false;
const _default = (mode)=>{
    const devClient = (0, _hotreloaderclient.default)(mode);
    devClient.subscribeToHmrEvent((obj)=>{
        if (reloading) return;
        // if we're on an error/404 page, we can't reliably tell if the newly added/removed page
        // matches the current path. In that case, assume any added/removed entries should trigger a reload of the current page
        const isOnErrorPage = window.next.router.pathname === '/404' || window.next.router.pathname === '/_error';
        switch(obj.action){
            case 'reloadPage':
                {
                    (0, _websocket.sendMessage)(JSON.stringify({
                        event: 'client-reload-page',
                        clientId: window.__nextDevClientId
                    }));
                    reloading = true;
                    return window.location.reload();
                }
            case 'removedPage':
                {
                    const [page] = obj.data;
                    if (page === window.next.router.pathname || isOnErrorPage) {
                        (0, _websocket.sendMessage)(JSON.stringify({
                            event: 'client-removed-page',
                            clientId: window.__nextDevClientId,
                            page
                        }));
                        return window.location.reload();
                    }
                    return;
                }
            case 'addedPage':
                {
                    const [page] = obj.data;
                    if (page === window.next.router.pathname && typeof window.next.router.components[page] === 'undefined' || isOnErrorPage) {
                        (0, _websocket.sendMessage)(JSON.stringify({
                            event: 'client-added-page',
                            clientId: window.__nextDevClientId,
                            page
                        }));
                        return window.location.reload();
                    }
                    return;
                }
            case 'serverError':
            case 'devPagesManifestUpdate':
            case 'appIsrManifest':
            case 'building':
            case 'finishBuilding':
                {
                    return;
                }
            default:
                {
                    throw new Error('Unexpected action ' + obj.action);
                }
        }
    });
    return devClient;
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=hot-middleware-client.js.map