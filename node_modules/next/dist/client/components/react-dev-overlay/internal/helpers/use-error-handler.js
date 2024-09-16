"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "useErrorHandler", {
    enumerable: true,
    get: function() {
        return useErrorHandler;
    }
});
const _react = require("react");
const _hydrationerrorinfo = require("./hydration-error-info");
const _isnextroutererror = require("../../../is-next-router-error");
const _ishydrationerror = require("../../../is-hydration-error");
if (typeof window !== "undefined") {
    try {
        // Increase the number of stack frames on the client
        Error.stackTraceLimit = 50;
    } catch (e) {}
}
let hasHydrationError = false;
const errorQueue = [];
const rejectionQueue = [];
const errorHandlers = [];
const rejectionHandlers = [];
if (typeof window !== "undefined") {
    // These event handlers must be added outside of the hook because there is no
    // guarantee that the hook will be alive in a mounted component in time to
    // when the errors occur.
    window.addEventListener("error", (ev)=>{
        if ((0, _isnextroutererror.isNextRouterError)(ev.error)) {
            ev.preventDefault();
            return;
        }
        const error = ev == null ? void 0 : ev.error;
        if (!error || !(error instanceof Error) || typeof error.stack !== "string") {
            // A non-error was thrown, we don't have anything to show. :-(
            return;
        }
        const isCausedByHydrationFailure = (0, _ishydrationerror.isHydrationError)(error);
        if ((0, _ishydrationerror.isHydrationError)(error) && !error.message.includes("https://nextjs.org/docs/messages/react-hydration-error")) {
            // If there's any extra information in the error message to display,
            // append it to the error message details property
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
        // Only queue one hydration every time
        if (isCausedByHydrationFailure) {
            if (!hasHydrationError) {
                errorQueue.push(e);
            }
            hasHydrationError = true;
        }
        for (const handler of errorHandlers){
            handler(e);
        }
    });
    window.addEventListener("unhandledrejection", (ev)=>{
        const reason = ev == null ? void 0 : ev.reason;
        if (!reason || !(reason instanceof Error) || typeof reason.stack !== "string") {
            // A non-error was thrown, we don't have anything to show. :-(
            return;
        }
        const e = reason;
        rejectionQueue.push(e);
        for (const handler of rejectionHandlers){
            handler(e);
        }
    });
}
function useErrorHandler(handleOnUnhandledError, handleOnUnhandledRejection) {
    (0, _react.useEffect)(()=>{
        // Handle queued errors.
        errorQueue.forEach(handleOnUnhandledError);
        rejectionQueue.forEach(handleOnUnhandledRejection);
        // Listen to new errors.
        errorHandlers.push(handleOnUnhandledError);
        rejectionHandlers.push(handleOnUnhandledRejection);
        return ()=>{
            // Remove listeners.
            errorHandlers.splice(errorHandlers.indexOf(handleOnUnhandledError), 1);
            rejectionHandlers.splice(rejectionHandlers.indexOf(handleOnUnhandledRejection), 1);
        };
    }, [
        handleOnUnhandledError,
        handleOnUnhandledRejection
    ]);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=use-error-handler.js.map