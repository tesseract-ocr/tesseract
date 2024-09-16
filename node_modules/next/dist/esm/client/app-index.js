import { jsx as _jsx } from "react/jsx-runtime";
import "../build/polyfills/polyfill-module";
// @ts-ignore react-dom/client exists when using React 18
import ReactDOMClient from "react-dom/client";
import React, { use } from "react";
// @ts-ignore
// eslint-disable-next-line import/no-extraneous-dependencies
import { createFromReadableStream } from "react-server-dom-webpack/client";
import { HeadManagerContext } from "../shared/lib/head-manager-context.shared-runtime";
import onRecoverableError from "./on-recoverable-error";
import { callServer } from "./app-call-server";
import { isNextRouterError } from "./components/is-next-router-error";
import { ActionQueueContext, createMutableActionQueue } from "../shared/lib/router/action-queue";
import { HMR_ACTIONS_SENT_TO_BROWSER } from "../server/dev/hot-reloader-types";
// Since React doesn't call onerror for errors caught in error boundaries.
const origConsoleError = window.console.error;
window.console.error = function() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    if (isNextRouterError(args[0])) {
        return;
    }
    origConsoleError.apply(window.console, args);
};
window.addEventListener("error", (ev)=>{
    if (isNextRouterError(ev.error)) {
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
const initialServerResponse = createFromReadableStream(readable, {
    callServer
});
function ServerRoot() {
    return use(initialServerResponse);
}
const StrictModeIfEnabled = process.env.__NEXT_STRICT_MODE_APP ? React.StrictMode : React.Fragment;
function Root(param) {
    let { children } = param;
    // TODO: remove in the next major version
    if (process.env.__NEXT_ANALYTICS_ID) {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        React.useEffect(()=>{
            require("./performance-relayer-app")();
        }, []);
    }
    if (process.env.__NEXT_TEST_MODE) {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        React.useEffect(()=>{
            window.__NEXT_HYDRATED = true;
            window.__NEXT_HYDRATED_CB == null ? void 0 : window.__NEXT_HYDRATED_CB.call(window);
        }, []);
    }
    return children;
}
export function hydrate() {
    const actionQueue = createMutableActionQueue();
    const reactEl = /*#__PURE__*/ _jsx(StrictModeIfEnabled, {
        children: /*#__PURE__*/ _jsx(HeadManagerContext.Provider, {
            value: {
                appDir: true
            },
            children: /*#__PURE__*/ _jsx(ActionQueueContext.Provider, {
                value: actionQueue,
                children: /*#__PURE__*/ _jsx(Root, {
                    children: /*#__PURE__*/ _jsx(ServerRoot, {})
                })
            })
        })
    });
    const rootLayoutMissingTags = window.__next_root_layout_missing_tags;
    const hasMissingTags = !!(rootLayoutMissingTags == null ? void 0 : rootLayoutMissingTags.length);
    const options = {
        onRecoverableError
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
                return /*#__PURE__*/ _jsx("html", {
                    id: "__next_error__",
                    children: /*#__PURE__*/ _jsx("body", {
                        children: children
                    })
                });
            } : React.Fragment;
            const errorTree = /*#__PURE__*/ _jsx(FallbackLayout, {
                children: /*#__PURE__*/ _jsx(ReactDevOverlay, {
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
                if (obj.action === HMR_ACTIONS_SENT_TO_BROWSER.SERVER_COMPONENT_CHANGES) {
                    window.location.reload();
                }
            };
            socket.addEventListener("message", handler);
            ReactDOMClient.createRoot(appElement, options).render(errorTree);
        } else {
            ReactDOMClient.createRoot(appElement, options).render(reactEl);
        }
    } else {
        React.startTransition(()=>ReactDOMClient.hydrateRoot(appElement, reactEl, {
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

//# sourceMappingURL=app-index.js.map