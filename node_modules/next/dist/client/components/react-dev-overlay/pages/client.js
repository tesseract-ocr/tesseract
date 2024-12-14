"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ReactDevOverlay: null,
    getErrorByType: null,
    getServerError: null,
    onBeforeRefresh: null,
    onBuildError: null,
    onBuildOk: null,
    onRefresh: null,
    onVersionInfo: null,
    register: null,
    unregister: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ReactDevOverlay: function() {
        return _ReactDevOverlay.default;
    },
    getErrorByType: function() {
        return _geterrorbytype.getErrorByType;
    },
    getServerError: function() {
        return _nodestackframes.getServerError;
    },
    onBeforeRefresh: function() {
        return onBeforeRefresh;
    },
    onBuildError: function() {
        return onBuildError;
    },
    onBuildOk: function() {
        return onBuildOk;
    },
    onRefresh: function() {
        return onRefresh;
    },
    onVersionInfo: function() {
        return onVersionInfo;
    },
    register: function() {
        return register;
    },
    unregister: function() {
        return unregister;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _bus = /*#__PURE__*/ _interop_require_wildcard._(require("./bus"));
const _parsestack = require("../internal/helpers/parse-stack");
const _parsecomponentstack = require("../internal/helpers/parse-component-stack");
const _hydrationerrorinfo = require("../internal/helpers/hydration-error-info");
const _shared = require("../shared");
const _attachhydrationerrorstate = require("../internal/helpers/attach-hydration-error-state");
const _geterrorbytype = require("../internal/helpers/get-error-by-type");
const _nodestackframes = require("../internal/helpers/node-stack-frames");
const _ReactDevOverlay = /*#__PURE__*/ _interop_require_default._(require("./ReactDevOverlay"));
let isRegistered = false;
let stackTraceLimit = undefined;
function handleError(error) {
    if (!error || !(error instanceof Error) || typeof error.stack !== 'string') {
        // A non-error was thrown, we don't have anything to show. :-(
        return;
    }
    (0, _attachhydrationerrorstate.attachHydrationErrorState)(error);
    const componentStackTrace = error._componentStack || _hydrationerrorinfo.hydrationErrorState.componentStack;
    const componentStackFrames = typeof componentStackTrace === 'string' ? (0, _parsecomponentstack.parseComponentStack)(componentStackTrace) : undefined;
    // Skip ModuleBuildError and ModuleNotFoundError, as it will be sent through onBuildError callback.
    // This is to avoid same error as different type showing up on client to cause flashing.
    if (error.name !== 'ModuleBuildError' && error.name !== 'ModuleNotFoundError') {
        _bus.emit({
            type: _shared.ACTION_UNHANDLED_ERROR,
            reason: error,
            frames: (0, _parsestack.parseStack)(error.stack),
            componentStackFrames
        });
    }
}
let origConsoleError = console.error;
function nextJsHandleConsoleError() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    // See https://github.com/facebook/react/blob/d50323eb845c5fde0d720cae888bf35dedd05506/packages/react-reconciler/src/ReactFiberErrorLogger.js#L78
    const error = process.env.NODE_ENV !== 'production' ? args[1] : args[0];
    (0, _hydrationerrorinfo.storeHydrationErrorStateFromConsoleArgs)(...args);
    handleError(error);
    origConsoleError.apply(window.console, args);
}
function onUnhandledError(event) {
    const error = event == null ? void 0 : event.error;
    handleError(error);
}
function onUnhandledRejection(ev) {
    const reason = ev == null ? void 0 : ev.reason;
    if (!reason || !(reason instanceof Error) || typeof reason.stack !== 'string') {
        // A non-error was thrown, we don't have anything to show. :-(
        return;
    }
    const e = reason;
    _bus.emit({
        type: _shared.ACTION_UNHANDLED_REJECTION,
        reason: reason,
        frames: (0, _parsestack.parseStack)(e.stack)
    });
}
function register() {
    if (isRegistered) {
        return;
    }
    isRegistered = true;
    try {
        const limit = Error.stackTraceLimit;
        Error.stackTraceLimit = 50;
        stackTraceLimit = limit;
    } catch (e) {}
    window.addEventListener('error', onUnhandledError);
    window.addEventListener('unhandledrejection', onUnhandledRejection);
    window.console.error = nextJsHandleConsoleError;
}
function unregister() {
    if (!isRegistered) {
        return;
    }
    isRegistered = false;
    if (stackTraceLimit !== undefined) {
        try {
            Error.stackTraceLimit = stackTraceLimit;
        } catch (e) {}
        stackTraceLimit = undefined;
    }
    window.removeEventListener('error', onUnhandledError);
    window.removeEventListener('unhandledrejection', onUnhandledRejection);
    window.console.error = origConsoleError;
}
function onBuildOk() {
    _bus.emit({
        type: _shared.ACTION_BUILD_OK
    });
}
function onBuildError(message) {
    _bus.emit({
        type: _shared.ACTION_BUILD_ERROR,
        message
    });
}
function onRefresh() {
    _bus.emit({
        type: _shared.ACTION_REFRESH
    });
}
function onBeforeRefresh() {
    _bus.emit({
        type: _shared.ACTION_BEFORE_REFRESH
    });
}
function onVersionInfo(versionInfo) {
    _bus.emit({
        type: _shared.ACTION_VERSION_INFO,
        versionInfo
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=client.js.map