"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ErrorHandlerSource: null,
    createErrorHandler: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ErrorHandlerSource: function() {
        return ErrorHandlerSource;
    },
    createErrorHandler: function() {
        return createErrorHandler;
    }
});
const _stringhash = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/string-hash"));
const _formatservererror = require("../../lib/format-server-error");
const _tracer = require("../lib/trace/tracer");
const _pipereadable = require("../pipe-readable");
const _isdynamicusageerror = require("../../export/helpers/is-dynamic-usage-error");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const ErrorHandlerSource = {
    serverComponents: "serverComponents",
    flightData: "flightData",
    html: "html"
};
function createErrorHandler({ /**
   * Used for debugging
   */ source, dev, isNextExport, errorLogger, digestErrorsMap, allCapturedErrors, silenceLogger }) {
    return (err, errorInfo)=>{
        var _err_message;
        // If the error already has a digest, respect the original digest,
        // so it won't get re-generated into another new error.
        if (!err.digest) {
            // TODO-APP: look at using webcrypto instead. Requires a promise to be awaited.
            err.digest = (0, _stringhash.default)(err.message + ((errorInfo == null ? void 0 : errorInfo.stack) || err.stack || "")).toString();
        }
        const digest = err.digest;
        if (allCapturedErrors) allCapturedErrors.push(err);
        // These errors are expected. We return the digest
        // so that they can be properly handled.
        if ((0, _isdynamicusageerror.isDynamicUsageError)(err)) return err.digest;
        // If the response was closed, we don't need to log the error.
        if ((0, _pipereadable.isAbortError)(err)) return;
        if (!digestErrorsMap.has(digest)) {
            digestErrorsMap.set(digest, err);
        } else if (source === ErrorHandlerSource.html) {
            // For SSR errors, if we have the existing digest in errors map,
            // we should use the existing error object to avoid duplicate error logs.
            err = digestErrorsMap.get(digest);
        }
        // Format server errors in development to add more helpful error messages
        if (dev) {
            (0, _formatservererror.formatServerError)(err);
        }
        // Used for debugging error source
        // console.error(source, err)
        // Don't log the suppressed error during export
        if (!(isNextExport && (err == null ? void 0 : (_err_message = err.message) == null ? void 0 : _err_message.includes("The specific message is omitted in production builds to avoid leaking sensitive details.")))) {
            // Record exception in an active span, if available.
            const span = (0, _tracer.getTracer)().getActiveScopeSpan();
            if (span) {
                span.recordException(err);
                span.setStatus({
                    code: _tracer.SpanStatusCode.ERROR,
                    message: err.message
                });
            }
            if (!silenceLogger) {
                if (errorLogger) {
                    errorLogger(err).catch(()=>{});
                } else {
                    // The error logger is currently not provided in the edge runtime.
                    // Use the exposed `__next_log_error__` instead.
                    // This will trace error traces to the original source code.
                    if (typeof __next_log_error__ === "function") {
                        __next_log_error__(err);
                    } else {
                        console.error(err);
                    }
                }
            }
        }
        return err.digest;
    };
}

//# sourceMappingURL=create-error-handler.js.map