"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "hydrate", {
    enumerable: true,
    get: function() {
        return hydrate;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _jsxruntime = require("react/jsx-runtime");
require("../build/polyfills/polyfill-module");
const _client = /*#__PURE__*/ _interop_require_default._(require("react-dom/client"));
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _client1 = require("react-server-dom-webpack/client");
const _headmanagercontextsharedruntime = require("../shared/lib/head-manager-context.shared-runtime");
const _onrecoverableerror = /*#__PURE__*/ _interop_require_default._(require("./on-recoverable-error"));
const _appcallserver = require("./app-call-server");
const _isnextroutererror = require("./components/is-next-router-error");
const _actionqueue = require("../shared/lib/router/action-queue");
const _hotreloadertypes = require("../server/dev/hot-reloader-types");
// Since React doesn't call onerror for errors caught in error boundaries.
const origConsoleError = window.console.error;
window.console.error = function() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    if ((0, _isnextroutererror.isNextRouterError)(args[0])) {
        return;
    }
    origConsoleError.apply(window.console, args);
};
window.addEventListener("error", (ev)=>{
    if ((0, _isnextroutererror.isNextRouterError)(ev.error)) {
        ev.preventDefault();
        return;
    }
});
/// <reference types="react-dom/experimental" />
const appElement = document;
const encoder = new TextEncoder();
let initialServerDataBuffer = undefined;
let initialServerDataWriter = undefined;
let initialServerDataLoaded = false;
let initialServerDataFlushed = false;
let initialFormStateData = null;
function nextServerDataCallback(seg) {
    if (seg[0] === 0) {
        initialServerDataBuffer = [];
    } else if (seg[0] === 1) {
        if (!initialServerDataBuffer) throw new Error("Unexpected server data: missing bootstrap script.");
        if (initialServerDataWriter) {
            initialServerDataWriter.enqueue(encoder.encode(seg[1]));
        } else {
            initialServerDataBuffer.push(seg[1]);
        }
    } else if (seg[0] === 2) {
        initialFormStateData = seg[1];
    }
}
// There might be race conditions between `nextServerDataRegisterWriter` and
// `DOMContentLoaded`. The former will be called when React starts to hydrate
// the root, the latter will be called when the DOM is fully loaded.
// For streaming, the former is called first due to partial hydration.
// For non-streaming, the latter can be called first.
// Hence, we use two variables `initialServerDataLoaded` and
// `initialServerDataFlushed` to make sure the writer will be closed and
// `initialServerDataBuffer` will be cleared in the right time.
function nextServerDataRegisterWriter(ctr) {
    if (initialServerDataBuffer) {
        initialServerDataBuffer.forEach((val)=>{
            ctr.enqueue(encoder.encode(val));
        });
        if (initialServerDataLoaded && !initialServerDataFlushed) {
            ctr.close();
            initialServerDataFlushed = true;
            initialServerDataBuffer = undefined;
        }
    }
    initialServerDataWriter = ctr;
}
// When `DOMContentLoaded`, we can close all pending writers to finish hydration.
const DOMContentLoaded = function() {
    if (initialServerDataWriter && !initialServerDataFlushed) {
        initialServerDataWriter.close();
        initialServerDataFlushed = true;
        initialServerDataBuffer = undefined;
    }
    initialServerDataLoaded = true;
};
// It's possible that the DOM is already loaded.
if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", DOMContentLoaded, false);
} else {
    DOMContentLoaded();
}
const nextServerDataLoadingGlobal = self.__next_f = self.__next_f || [];
nextServerDataLoadingGlobal.forEach(nextServerDataCallback);
nextServerDataLoadingGlobal.push = nextServerDataCallback;
const readable = new ReadableStream({
    start (controller) {
        nextServerDataRegisterWriter(controller);
    }
});
const initialServerResponse = (0, _client1.createFromReadableStream)(readable, {
    callServer: _appcallserver.callServer
});
function ServerRoot() {
    return (0, _react.use)(initialServerResponse);
}
const StrictModeIfEnabled = process.env.__NEXT_STRICT_MODE_APP ? _react.default.StrictMode : _react.default.Fragment;
function Root(param) {
    let { children } = param;
    // TODO: remove in the next major version
    if (process.env.__NEXT_ANALYTICS_ID) {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        _react.default.useEffect(()=>{
            require("./performance-relayer-app")();
        }, []);
    }
    if (process.env.__NEXT_TEST_MODE) {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        _react.default.useEffect(()=>{
            window.__NEXT_HYDRATED = true;
            window.__NEXT_HYDRATED_CB == null ? void 0 : window.__NEXT_HYDRATED_CB.call(window);
        }, []);
    }
    return children;
}
function hydrate() {
    const actionQueue = (0, _actionqueue.createMutableActionQueue)();
    const reactEl = /*#__PURE__*/ (0, _jsxruntime.jsx)(StrictModeIfEnabled, {
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_headmanagercontextsharedruntime.HeadManagerContext.Provider, {
            value: {
                appDir: true
            },
            children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_actionqueue.ActionQueueContext.Provider, {
                value: actionQueue,
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(Root, {
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)(ServerRoot, {})
                })
            })
        })
    });
    const rootLayoutMissingTags = window.__next_root_layout_missing_tags;
    const hasMissingTags = !!(rootLayoutMissingTags == null ? void 0 : rootLayoutMissingTags.length);
    const options = {
        onRecoverableError: _onrecoverableerror.default
    };
    const isError = document.documentElement.id === "__next_error__" || hasMissingTags;
    if (process.env.NODE_ENV !== "production") {
        // Patch console.error to collect information about hydration errors
        const patchConsoleError = require("./components/react-dev-overlay/internal/helpers/hydration-error-info").patchConsoleError;
        if (!isError) {
            patchConsoleError();
        }
    }
    if (isError) {
        if (process.env.NODE_ENV !== "production") {
            // if an error is thrown while rendering an RSC stream, this will catch it in dev
            // and show the error overlay
            const ReactDevOverlay = require("./components/react-dev-overlay/app/ReactDevOverlay").default;
            const INITIAL_OVERLAY_STATE = require("./components/react-dev-overlay/shared").INITIAL_OVERLAY_STATE;
            const getSocketUrl = require("./components/react-dev-overlay/internal/helpers/get-socket-url").getSocketUrl;
            const FallbackLayout = hasMissingTags ? (param)=>{
                let { children } = param;
                return /*#__PURE__*/ (0, _jsxruntime.jsx)("html", {
                    id: "__next_error__",
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)("body", {
                        children: children
                    })
                });
            } : _react.default.Fragment;
            const errorTree = /*#__PURE__*/ (0, _jsxruntime.jsx)(FallbackLayout, {
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(ReactDevOverlay, {
                    state: {
                        ...INITIAL_OVERLAY_STATE,
                        rootLayoutMissingTags
                    },
                    onReactError: ()=>{},
                    children: reactEl
                })
            });
            const socketUrl = getSocketUrl(process.env.__NEXT_ASSET_PREFIX || "");
            const socket = new window.WebSocket("" + socketUrl + "/_next/webpack-hmr");
            // add minimal "hot reload" support for RSC errors
            const handler = (event)=>{
                let obj;
                try {
                    obj = JSON.parse(event.data);
                } catch (e) {}
                if (!obj || !("action" in obj)) {
                    return;
                }
                if (obj.action === _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_COMPONENT_CHANGES) {
                    window.location.reload();
                }
            };
            socket.addEventListener("message", handler);
            _client.default.createRoot(appElement, options).render(errorTree);
        } else {
            _client.default.createRoot(appElement, options).render(reactEl);
        }
    } else {
        _react.default.startTransition(()=>_client.default.hydrateRoot(appElement, reactEl, {
                ...options,
                formState: initialFormStateData
            }));
    }
    // TODO-APP: Remove this logic when Float has GC built-in in development.
    if (process.env.NODE_ENV !== "production") {
        const { linkGc } = require("./app-link-gc");
        linkGc();
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=app-index.js.map