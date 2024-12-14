// imports polyfill from `@next/polyfill-module` after build.
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
require("./components/globals/patch-console");
require("./components/globals/handle-global-errors");
const _client = /*#__PURE__*/ _interop_require_default._(require("react-dom/client"));
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _client1 = require("react-server-dom-webpack/client");
const _headmanagercontextsharedruntime = require("../shared/lib/head-manager-context.shared-runtime");
const _shared = require("./react-client-callbacks/shared");
const _approuter = require("./react-client-callbacks/app-router");
const _appcallserver = require("./app-call-server");
const _appfindsourcemapurl = require("./app-find-source-map-url");
const _actionqueue = require("../shared/lib/router/action-queue");
const _approuter1 = /*#__PURE__*/ _interop_require_default._(require("./components/app-router"));
const _createinitialrouterstate = require("./components/router-reducer/create-initial-router-state");
const _approutercontextsharedruntime = require("../shared/lib/app-router-context.shared-runtime");
const _appbuildid = require("./app-build-id");
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
        if (!initialServerDataBuffer) throw new Error('Unexpected server data: missing bootstrap script.');
        if (initialServerDataWriter) {
            initialServerDataWriter.enqueue(encoder.encode(seg[1]));
        } else {
            initialServerDataBuffer.push(seg[1]);
        }
    } else if (seg[0] === 2) {
        initialFormStateData = seg[1];
    } else if (seg[0] === 3) {
        if (!initialServerDataBuffer) throw new Error('Unexpected server data: missing bootstrap script.');
        // Decode the base64 string back to binary data.
        const binaryString = atob(seg[1]);
        const decodedChunk = new Uint8Array(binaryString.length);
        for(var i = 0; i < binaryString.length; i++){
            decodedChunk[i] = binaryString.charCodeAt(i);
        }
        if (initialServerDataWriter) {
            initialServerDataWriter.enqueue(decodedChunk);
        } else {
            initialServerDataBuffer.push(decodedChunk);
        }
    }
}
function isStreamErrorOrUnfinished(ctr) {
    // If `desiredSize` is null, it means the stream is closed or errored. If it is lower than 0, the stream is still unfinished.
    return ctr.desiredSize === null || ctr.desiredSize < 0;
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
            ctr.enqueue(typeof val === 'string' ? encoder.encode(val) : val);
        });
        if (initialServerDataLoaded && !initialServerDataFlushed) {
            if (isStreamErrorOrUnfinished(ctr)) {
                ctr.error(new Error('The connection to the page was unexpectedly closed, possibly due to the stop button being clicked, loss of Wi-Fi, or an unstable internet connection.'));
            } else {
                ctr.close();
            }
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
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', DOMContentLoaded, false);
} else {
    // Delayed in marco task to ensure it's executed later than hydration
    setTimeout(DOMContentLoaded);
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
    callServer: _appcallserver.callServer,
    findSourceMapURL: _appfindsourcemapurl.findSourceMapURL
});
// React overrides `.then` and doesn't return a new promise chain,
// so we wrap the action queue in a promise to ensure that its value
// is defined when the promise resolves.
// https://github.com/facebook/react/blob/163365a07872337e04826c4f501565d43dbd2fd4/packages/react-client/src/ReactFlightClient.js#L189-L190
const pendingActionQueue = new Promise((resolve, reject)=>{
    initialServerResponse.then((initialRSCPayload)=>{
        // setAppBuildId should be called only once, during JS initialization
        // and before any components have hydrated.
        (0, _appbuildid.setAppBuildId)(initialRSCPayload.b);
        resolve((0, _actionqueue.createMutableActionQueue)((0, _createinitialrouterstate.createInitialRouterState)({
            initialFlightData: initialRSCPayload.f,
            initialCanonicalUrlParts: initialRSCPayload.c,
            initialParallelRoutes: new Map(),
            location: window.location,
            couldBeIntercepted: initialRSCPayload.i,
            postponed: initialRSCPayload.s,
            prerendered: initialRSCPayload.S
        })));
    }, (err)=>reject(err));
});
function ServerRoot() {
    const initialRSCPayload = (0, _react.use)(initialServerResponse);
    const actionQueue = (0, _react.use)(pendingActionQueue);
    const router = /*#__PURE__*/ (0, _jsxruntime.jsx)(_approuter1.default, {
        actionQueue: actionQueue,
        globalErrorComponentAndStyles: initialRSCPayload.G,
        assetPrefix: initialRSCPayload.p
    });
    if (process.env.NODE_ENV === 'development' && initialRSCPayload.m) {
        // We provide missing slot information in a context provider only during development
        // as we log some additional information about the missing slots in the console.
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(_approutercontextsharedruntime.MissingSlotContext, {
            value: initialRSCPayload.m,
            children: router
        });
    }
    return router;
}
const StrictModeIfEnabled = process.env.__NEXT_STRICT_MODE_APP ? _react.default.StrictMode : _react.default.Fragment;
function Root(param) {
    let { children } = param;
    if (process.env.__NEXT_TEST_MODE) {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        _react.default.useEffect(()=>{
            window.__NEXT_HYDRATED = true;
            window.__NEXT_HYDRATED_CB == null ? void 0 : window.__NEXT_HYDRATED_CB.call(window);
        }, []);
    }
    return children;
}
const reactRootOptions = {
    onRecoverableError: _shared.onRecoverableError,
    onCaughtError: _approuter.onCaughtError,
    onUncaughtError: _approuter.onUncaughtError
};
function hydrate() {
    const reactEl = /*#__PURE__*/ (0, _jsxruntime.jsx)(StrictModeIfEnabled, {
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_headmanagercontextsharedruntime.HeadManagerContext.Provider, {
            value: {
                appDir: true
            },
            children: /*#__PURE__*/ (0, _jsxruntime.jsx)(Root, {
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(ServerRoot, {})
            })
        })
    });
    const rootLayoutMissingTags = window.__next_root_layout_missing_tags;
    const hasMissingTags = !!(rootLayoutMissingTags == null ? void 0 : rootLayoutMissingTags.length);
    const isError = document.documentElement.id === '__next_error__' || hasMissingTags;
    if (isError) {
        if (process.env.NODE_ENV !== 'production') {
            const createDevOverlayElement = require('./components/react-dev-overlay/client-entry').createDevOverlayElement;
            const errorTree = createDevOverlayElement(reactEl);
            _client.default.createRoot(appElement, reactRootOptions).render(errorTree);
        } else {
            _client.default.createRoot(appElement, reactRootOptions).render(reactEl);
        }
    } else {
        _react.default.startTransition(()=>_client.default.hydrateRoot(appElement, reactEl, {
                ...reactRootOptions,
                formState: initialFormStateData
            }));
    }
    // TODO-APP: Remove this logic when Float has GC built-in in development.
    if (process.env.NODE_ENV !== 'production') {
        const { linkGc } = require('./app-link-gc');
        linkGc();
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=app-index.js.map