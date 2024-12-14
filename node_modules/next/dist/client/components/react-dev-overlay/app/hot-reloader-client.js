"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    waitForWebpackRuntimeHotUpdate: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return HotReload;
    },
    waitForWebpackRuntimeHotUpdate: function() {
        return waitForWebpackRuntimeHotUpdate;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _jsxruntime = require("react/jsx-runtime");
const _react = require("react");
const _stripansi = /*#__PURE__*/ _interop_require_default._(require("next/dist/compiled/strip-ansi"));
const _formatwebpackmessages = /*#__PURE__*/ _interop_require_default._(require("../internal/helpers/format-webpack-messages"));
const _navigation = require("../../navigation");
const _shared = require("../shared");
const _parsestack = require("../internal/helpers/parse-stack");
const _ReactDevOverlay = /*#__PURE__*/ _interop_require_default._(require("./ReactDevOverlay"));
const _useerrorhandler = require("../internal/helpers/use-error-handler");
const _runtimeerrorhandler = require("../internal/helpers/runtime-error-handler");
const _usewebsocket = require("../internal/helpers/use-websocket");
const _parsecomponentstack = require("../internal/helpers/parse-component-stack");
const _hotreloadertypes = require("../../../../server/dev/hot-reloader-types");
const _extractmodulesfromturbopackmessage = require("../../../../server/dev/extract-modules-from-turbopack-message");
const _navigationuntracked = require("../../navigation-untracked");
const _stitchederror = require("../internal/helpers/stitched-error");
let mostRecentCompilationHash = null;
let __nextDevClientId = Math.round(Math.random() * 100 + Date.now());
let reloading = false;
let startLatency = null;
let pendingHotUpdateWebpack = Promise.resolve();
let resolvePendingHotUpdateWebpack = ()=>{};
function setPendingHotUpdateWebpack() {
    pendingHotUpdateWebpack = new Promise((resolve)=>{
        resolvePendingHotUpdateWebpack = ()=>{
            resolve();
        };
    });
}
function waitForWebpackRuntimeHotUpdate() {
    return pendingHotUpdateWebpack;
}
function handleBeforeHotUpdateWebpack(dispatcher, hasUpdates) {
    if (hasUpdates) {
        dispatcher.onBeforeRefresh();
    }
}
function handleSuccessfulHotUpdateWebpack(dispatcher, sendMessage, updatedModules) {
    resolvePendingHotUpdateWebpack();
    dispatcher.onBuildOk();
    reportHmrLatency(sendMessage, updatedModules);
    dispatcher.onRefresh();
}
function reportHmrLatency(sendMessage, updatedModules) {
    if (!startLatency) return;
    let endLatency = Date.now();
    const latency = endLatency - startLatency;
    console.log("[Fast Refresh] done in " + latency + "ms");
    sendMessage(JSON.stringify({
        event: 'client-hmr-latency',
        id: window.__nextDevClientId,
        startTime: startLatency,
        endTime: endLatency,
        page: window.location.pathname,
        updatedModules,
        // Whether the page (tab) was hidden at the time the event occurred.
        // This can impact the accuracy of the event's timing.
        isPageHidden: document.visibilityState === 'hidden'
    }));
}
// There is a newer version of the code available.
function handleAvailableHash(hash) {
    // Update last known compilation hash.
    mostRecentCompilationHash = hash;
}
/**
 * Is there a newer version of this code available?
 * For webpack: Check if the hash changed compared to __webpack_hash__
 * For Turbopack: Always true because it doesn't have __webpack_hash__
 */ function isUpdateAvailable() {
    if (process.env.TURBOPACK) {
        return true;
    }
    /* globals __webpack_hash__ */ // __webpack_hash__ is the hash of the current compilation.
    // It's a global variable injected by Webpack.
    return mostRecentCompilationHash !== __webpack_hash__;
}
// Webpack disallows updates in other states.
function canApplyUpdates() {
    // @ts-expect-error module.hot exists
    return module.hot.status() === 'idle';
}
function afterApplyUpdates(fn) {
    if (canApplyUpdates()) {
        fn();
    } else {
        function handler(status) {
            if (status === 'idle') {
                // @ts-expect-error module.hot exists
                module.hot.removeStatusHandler(handler);
                fn();
            }
        }
        // @ts-expect-error module.hot exists
        module.hot.addStatusHandler(handler);
    }
}
function performFullReload(err, sendMessage) {
    const stackTrace = err && (err.stack && err.stack.split('\n').slice(0, 5).join('\n') || err.message || err + '');
    sendMessage(JSON.stringify({
        event: 'client-full-reload',
        stackTrace,
        hadRuntimeError: !!_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError,
        dependencyChain: err ? err.dependencyChain : undefined
    }));
    if (reloading) return;
    reloading = true;
    window.location.reload();
}
// Attempt to update code on the fly, fall back to a hard reload.
function tryApplyUpdates(onBeforeUpdate, onHotUpdateSuccess, sendMessage, dispatcher) {
    if (!isUpdateAvailable() || !canApplyUpdates()) {
        resolvePendingHotUpdateWebpack();
        dispatcher.onBuildOk();
        reportHmrLatency(sendMessage, []);
        return;
    }
    function handleApplyUpdates(err, updatedModules) {
        if (err || _runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError || !updatedModules) {
            if (err) {
                console.warn('[Fast Refresh] performing full reload\n\n' + "Fast Refresh will perform a full reload when you edit a file that's imported by modules outside of the React rendering tree.\n" + 'You might have a file which exports a React component but also exports a value that is imported by a non-React component file.\n' + 'Consider migrating the non-React component export to a separate file and importing it into both files.\n\n' + 'It is also possible the parent component of the component you edited is a class component, which disables Fast Refresh.\n' + 'Fast Refresh requires at least one parent function component in your React tree.');
            } else if (_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError) {
                console.warn(_shared.REACT_REFRESH_FULL_RELOAD_FROM_ERROR);
            }
            performFullReload(err, sendMessage);
            return;
        }
        const hasUpdates = Boolean(updatedModules.length);
        if (typeof onHotUpdateSuccess === 'function') {
            // Maybe we want to do something.
            onHotUpdateSuccess(updatedModules);
        }
        if (isUpdateAvailable()) {
            // While we were updating, there was a new update! Do it again.
            tryApplyUpdates(hasUpdates ? ()=>{} : onBeforeUpdate, hasUpdates ? ()=>dispatcher.onBuildOk() : onHotUpdateSuccess, sendMessage, dispatcher);
        } else {
            dispatcher.onBuildOk();
            if (process.env.__NEXT_TEST_MODE) {
                afterApplyUpdates(()=>{
                    if (self.__NEXT_HMR_CB) {
                        self.__NEXT_HMR_CB();
                        self.__NEXT_HMR_CB = null;
                    }
                });
            }
        }
    }
    // https://webpack.js.org/api/hot-module-replacement/#check
    // @ts-expect-error module.hot exists
    module.hot.check(/* autoApply */ false).then((updatedModules)=>{
        if (!updatedModules) {
            return null;
        }
        if (typeof onBeforeUpdate === 'function') {
            const hasUpdates = Boolean(updatedModules.length);
            onBeforeUpdate(hasUpdates);
        }
        // https://webpack.js.org/api/hot-module-replacement/#apply
        // @ts-expect-error module.hot exists
        return module.hot.apply();
    }).then((updatedModules)=>{
        handleApplyUpdates(null, updatedModules);
    }, (err)=>{
        handleApplyUpdates(err, null);
    });
}
/** Handles messages from the sevrer for the App Router. */ function processMessage(obj, sendMessage, processTurbopackMessage, router, dispatcher, appIsrManifestRef, pathnameRef) {
    if (!('action' in obj)) {
        return;
    }
    function handleErrors(errors) {
        // "Massage" webpack messages.
        const formatted = (0, _formatwebpackmessages.default)({
            errors: errors,
            warnings: []
        });
        // Only show the first error.
        dispatcher.onBuildError(formatted.errors[0]);
        // Also log them to the console.
        for(let i = 0; i < formatted.errors.length; i++){
            console.error((0, _stripansi.default)(formatted.errors[i]));
        }
        // Do not attempt to reload now.
        // We will reload on next success instead.
        if (process.env.__NEXT_TEST_MODE) {
            if (self.__NEXT_HMR_CB) {
                self.__NEXT_HMR_CB(formatted.errors[0]);
                self.__NEXT_HMR_CB = null;
            }
        }
    }
    function handleHotUpdate() {
        if (process.env.TURBOPACK) {
            dispatcher.onBuildOk();
        } else {
            tryApplyUpdates(function onBeforeHotUpdate(hasUpdates) {
                handleBeforeHotUpdateWebpack(dispatcher, hasUpdates);
            }, function onSuccessfulHotUpdate(webpackUpdatedModules) {
                // Only dismiss it when we're sure it's a hot update.
                // Otherwise it would flicker right before the reload.
                handleSuccessfulHotUpdateWebpack(dispatcher, sendMessage, webpackUpdatedModules);
            }, sendMessage, dispatcher);
        }
    }
    switch(obj.action){
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.APP_ISR_MANIFEST:
            {
                if (process.env.__NEXT_APP_ISR_INDICATOR) {
                    if (appIsrManifestRef) {
                        appIsrManifestRef.current = obj.data;
                        // handle initial status on receiving manifest
                        // navigation is handled in useEffect for pathname changes
                        // as we'll receive the updated manifest before usePathname
                        // triggers for new value
                        if (pathnameRef.current in obj.data) {
                            var _localStorage;
                            // the indicator can be hidden for an hour.
                            // check if it's still hidden
                            const indicatorHiddenAt = Number((_localStorage = localStorage) == null ? void 0 : _localStorage.getItem('__NEXT_DISMISS_PRERENDER_INDICATOR'));
                            const isHidden = indicatorHiddenAt && !isNaN(indicatorHiddenAt) && Date.now() < indicatorHiddenAt;
                            if (!isHidden) {
                                dispatcher.onStaticIndicator(true);
                            }
                        } else {
                            dispatcher.onStaticIndicator(false);
                        }
                    }
                }
                break;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.BUILDING:
            {
                startLatency = Date.now();
                if (!process.env.TURBOPACK) {
                    setPendingHotUpdateWebpack();
                }
                console.log('[Fast Refresh] rebuilding');
                break;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.BUILT:
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SYNC:
            {
                if (obj.hash) {
                    handleAvailableHash(obj.hash);
                }
                const { errors, warnings } = obj;
                // Is undefined when it's a 'built' event
                if ('versionInfo' in obj) dispatcher.onVersionInfo(obj.versionInfo);
                if ('debug' in obj && obj.debug) dispatcher.onDebugInfo(obj.debug);
                const hasErrors = Boolean(errors && errors.length);
                // Compilation with errors (e.g. syntax error or missing modules).
                if (hasErrors) {
                    sendMessage(JSON.stringify({
                        event: 'client-error',
                        errorCount: errors.length,
                        clientId: __nextDevClientId
                    }));
                    handleErrors(errors);
                    return;
                }
                const hasWarnings = Boolean(warnings && warnings.length);
                if (hasWarnings) {
                    sendMessage(JSON.stringify({
                        event: 'client-warning',
                        warningCount: warnings.length,
                        clientId: __nextDevClientId
                    }));
                    // Print warnings to the console.
                    const formattedMessages = (0, _formatwebpackmessages.default)({
                        warnings: warnings,
                        errors: []
                    });
                    for(let i = 0; i < formattedMessages.warnings.length; i++){
                        if (i === 5) {
                            console.warn('There were more warnings in other files.\n' + 'You can find a complete log in the terminal.');
                            break;
                        }
                        console.warn((0, _stripansi.default)(formattedMessages.warnings[i]));
                    }
                // No early return here as we need to apply modules in the same way between warnings only and compiles without warnings
                }
                sendMessage(JSON.stringify({
                    event: 'client-success',
                    clientId: __nextDevClientId
                }));
                if (obj.action === _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.BUILT) {
                    // Handle hot updates
                    handleHotUpdate();
                }
                return;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_CONNECTED:
            {
                processTurbopackMessage({
                    type: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_CONNECTED,
                    data: {
                        sessionId: obj.data.sessionId
                    }
                });
                break;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_MESSAGE:
            {
                const updatedModules = (0, _extractmodulesfromturbopackmessage.extractModulesFromTurbopackMessage)(obj.data);
                dispatcher.onBeforeRefresh();
                processTurbopackMessage({
                    type: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_MESSAGE,
                    data: obj.data
                });
                dispatcher.onRefresh();
                if (_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError) {
                    console.warn(_shared.REACT_REFRESH_FULL_RELOAD_FROM_ERROR);
                    performFullReload(null, sendMessage);
                }
                reportHmrLatency(sendMessage, updatedModules);
                break;
            }
        // TODO-APP: make server component change more granular
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_COMPONENT_CHANGES:
            {
                sendMessage(JSON.stringify({
                    event: 'server-component-reload-page',
                    clientId: __nextDevClientId
                }));
                if (_runtimeerrorhandler.RuntimeErrorHandler.hadRuntimeError) {
                    if (reloading) return;
                    reloading = true;
                    return window.location.reload();
                }
                (0, _react.startTransition)(()=>{
                    router.hmrRefresh();
                    dispatcher.onRefresh();
                });
                if (process.env.__NEXT_TEST_MODE) {
                    if (self.__NEXT_HMR_CB) {
                        self.__NEXT_HMR_CB();
                        self.__NEXT_HMR_CB = null;
                    }
                }
                return;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.RELOAD_PAGE:
            {
                sendMessage(JSON.stringify({
                    event: 'client-reload-page',
                    clientId: __nextDevClientId
                }));
                if (reloading) return;
                reloading = true;
                return window.location.reload();
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.ADDED_PAGE:
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.REMOVED_PAGE:
            {
                // TODO-APP: potentially only refresh if the currently viewed page was added/removed.
                return router.hmrRefresh();
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_ERROR:
            {
                const { errorJSON } = obj;
                if (errorJSON) {
                    const { message, stack } = JSON.parse(errorJSON);
                    const error = new Error(message);
                    error.stack = stack;
                    handleErrors([
                        error
                    ]);
                }
                return;
            }
        case _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.DEV_PAGES_MANIFEST_UPDATE:
            {
                return;
            }
        default:
            {}
    }
}
function HotReload(param) {
    let { assetPrefix, children } = param;
    const [state, dispatch] = (0, _shared.useErrorOverlayReducer)();
    const dispatcher = (0, _react.useMemo)(()=>{
        return {
            onBuildOk () {
                dispatch({
                    type: _shared.ACTION_BUILD_OK
                });
            },
            onBuildError (message) {
                dispatch({
                    type: _shared.ACTION_BUILD_ERROR,
                    message
                });
            },
            onBeforeRefresh () {
                dispatch({
                    type: _shared.ACTION_BEFORE_REFRESH
                });
            },
            onRefresh () {
                dispatch({
                    type: _shared.ACTION_REFRESH
                });
            },
            onVersionInfo (versionInfo) {
                dispatch({
                    type: _shared.ACTION_VERSION_INFO,
                    versionInfo
                });
            },
            onStaticIndicator (status) {
                dispatch({
                    type: _shared.ACTION_STATIC_INDICATOR,
                    staticIndicator: status
                });
            },
            onDebugInfo (debugInfo) {
                dispatch({
                    type: _shared.ACTION_DEBUG_INFO,
                    debugInfo
                });
            }
        };
    }, [
        dispatch
    ]);
    const handleOnUnhandledError = (0, _react.useCallback)((error)=>{
        const errorDetails = error.details;
        // Component stack is added to the error in use-error-handler in case there was a hydration error
        const componentStackTrace = error._componentStack || (errorDetails == null ? void 0 : errorDetails.componentStack);
        const warning = errorDetails == null ? void 0 : errorDetails.warning;
        const stitchedError = (0, _stitchederror.getReactStitchedError)(error);
        dispatch({
            type: _shared.ACTION_UNHANDLED_ERROR,
            reason: stitchedError,
            frames: (0, _parsestack.parseStack)(stitchedError.stack || ''),
            componentStackFrames: typeof componentStackTrace === 'string' ? (0, _parsecomponentstack.parseComponentStack)(componentStackTrace) : undefined,
            warning
        });
    }, [
        dispatch
    ]);
    const handleOnUnhandledRejection = (0, _react.useCallback)((reason)=>{
        const stitchedError = (0, _stitchederror.getReactStitchedError)(reason);
        dispatch({
            type: _shared.ACTION_UNHANDLED_REJECTION,
            reason: stitchedError,
            frames: (0, _parsestack.parseStack)(stitchedError.stack || '')
        });
    }, [
        dispatch
    ]);
    (0, _useerrorhandler.useErrorHandler)(handleOnUnhandledError, handleOnUnhandledRejection);
    const webSocketRef = (0, _usewebsocket.useWebsocket)(assetPrefix);
    (0, _usewebsocket.useWebsocketPing)(webSocketRef);
    const sendMessage = (0, _usewebsocket.useSendMessage)(webSocketRef);
    const processTurbopackMessage = (0, _usewebsocket.useTurbopack)(sendMessage, (err)=>performFullReload(err, sendMessage));
    const router = (0, _navigation.useRouter)();
    // We don't want access of the pathname for the dev tools to trigger a dynamic
    // access (as the dev overlay will never be present in production).
    const pathname = (0, _navigationuntracked.useUntrackedPathname)();
    const appIsrManifestRef = (0, _react.useRef)({});
    const pathnameRef = (0, _react.useRef)(pathname);
    if (process.env.__NEXT_APP_ISR_INDICATOR) {
        // this conditional is only for dead-code elimination which
        // isn't a runtime conditional only build-time so ignore hooks rule
        // eslint-disable-next-line react-hooks/rules-of-hooks
        (0, _react.useEffect)(()=>{
            pathnameRef.current = pathname;
            const appIsrManifest = appIsrManifestRef.current;
            if (appIsrManifest) {
                if (pathname && pathname in appIsrManifest) {
                    try {
                        var _localStorage;
                        const indicatorHiddenAt = Number((_localStorage = localStorage) == null ? void 0 : _localStorage.getItem('__NEXT_DISMISS_PRERENDER_INDICATOR'));
                        const isHidden = indicatorHiddenAt && !isNaN(indicatorHiddenAt) && Date.now() < indicatorHiddenAt;
                        if (!isHidden) {
                            dispatcher.onStaticIndicator(true);
                        }
                    } catch (reason) {
                        let message = '';
                        if (reason instanceof DOMException) {
                            var _reason_stack;
                            // Most likely a SecurityError, because of an unavailable localStorage
                            message = (_reason_stack = reason.stack) != null ? _reason_stack : reason.message;
                        } else if (reason instanceof Error) {
                            var _reason_stack1;
                            message = 'Error: ' + reason.message + '\n' + ((_reason_stack1 = reason.stack) != null ? _reason_stack1 : '');
                        } else {
                            message = 'Unexpected Exception: ' + reason;
                        }
                        console.warn('[HMR] ' + message);
                    }
                } else {
                    dispatcher.onStaticIndicator(false);
                }
            }
        }, [
            pathname,
            dispatcher
        ]);
    }
    (0, _react.useEffect)(()=>{
        const websocket = webSocketRef.current;
        if (!websocket) return;
        const handler = (event)=>{
            try {
                const obj = JSON.parse(event.data);
                processMessage(obj, sendMessage, processTurbopackMessage, router, dispatcher, appIsrManifestRef, pathnameRef);
            } catch (err) {
                var _err_stack;
                console.warn('[HMR] Invalid message: ' + JSON.stringify(event.data) + '\n' + ((_err_stack = err == null ? void 0 : err.stack) != null ? _err_stack : ''));
            }
        };
        websocket.addEventListener('message', handler);
        return ()=>websocket.removeEventListener('message', handler);
    }, [
        sendMessage,
        router,
        webSocketRef,
        dispatcher,
        processTurbopackMessage,
        appIsrManifestRef
    ]);
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_ReactDevOverlay.default, {
        state: state,
        dispatcher: dispatcher,
        children: children
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=hot-reloader-client.js.map