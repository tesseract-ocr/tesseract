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
        return _getErrorByType.getErrorByType;
    },
    getServerError: function() {
        return _nodeStackFrames.getServerError;
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
const _parseStack = require("../internal/helpers/parseStack");
const _parsecomponentstack = require("../internal/helpers/parse-component-stack");
const _hydrationerrorinfo = require("../internal/helpers/hydration-error-info");
const _shared = require("../shared");
const _getErrorByType = require("../internal/helpers/getErrorByType");
const _nodeStackFrames = require("../internal/helpers/nodeStackFrames");
const _ReactDevOverlay = /*#__PURE__*/ _interop_require_default._(require("./ReactDevOverlay"));
// Patch console.error to collect information about hydration errors
(0, _hydrationerrorinfo.patchConsoleError)();
let isRegistered = false;
let stackTraceLimit = undefined;
function onUnhandledError(ev) {
    const error = ev == null ? void 0 : ev.error;
    if (!error || !(error instanceof Error) || typeof error.stack !== "string") {
        // A non-error was thrown, we don't have anything to show. :-(
        return;
    }
    if (error.message.match(/(hydration|content does not match|did not match)/i)) {
        if (_hydrationerrorinfo.hydrationErrorState.warning) {
            error.details = {
                ...error.details,
                // It contains the warning, component stack, server and client tag names
                ..._hydrationerrorinfo.hydrationErrorState
            };
        }
        error.message += "\nSee more info here: https://nextjs.org/docs/messages/react-hydration-error";
    }
    const e = error;
    const componentStackFrames = typeof _hydrationerrorinfo.hydrationErrorState.componentStack === "string" ? (0, _parsecomponentstack.parseComponentStack)(_hydrationerrorinfo.hydrationErrorState.componentStack) : undefined;
    // Skip ModuleBuildError and ModuleNotFoundError, as it will be sent through onBuildError callback.
    // This is to avoid same error as different type showing up on client to cause flashing.
    if (e.name !== "ModuleBuildError" && e.name !== "ModuleNotFoundError") {
        _bus.emit({
            type: _shared.ACTION_UNHANDLED_ERROR,
            reason: error,
            frames: (0, _parseStack.parseStack)(e.stack),
            componentStackFrames
        });
    }
}
function onUnhandledRejection(ev) {
    const reason = ev == null ? void 0 : ev.reason;
    if (!reason || !(reason instanceof Error) || typeof reason.stack !== "string") {
        // A non-error was thrown, we don't have anything to show. :-(
        return;
    }
    const e = reason;
    _bus.emit({
        type: _shared.ACTION_UNHANDLED_REJECTION,
        reason: reason,
        frames: (0, _parseStack.parseStack)(e.stack)
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
    window.addEventListener("error", onUnhandledError);
    window.addEventListener("unhandledrejection", onUnhandledRejection);
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
    window.removeEventListener("error", onUnhandledError);
    window.removeEventListener("unhandledrejection", onUnhandledRejection);
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