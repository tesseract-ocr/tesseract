"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createFlightReactServerErrorHandler: null,
    createHTMLErrorHandler: null,
    createHTMLReactServerErrorHandler: null,
    getDigestForWellKnownError: null,
    isUserLandError: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createFlightReactServerErrorHandler: function() {
        return createFlightReactServerErrorHandler;
    },
    createHTMLErrorHandler: function() {
        return createHTMLErrorHandler;
    },
    createHTMLReactServerErrorHandler: function() {
        return createHTMLReactServerErrorHandler;
    },
    getDigestForWellKnownError: function() {
        return getDigestForWellKnownError;
    },
    isUserLandError: function() {
        return isUserLandError;
    }
});
const _stringhash = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/string-hash"));
const _formatservererror = require("../../lib/format-server-error");
const _tracer = require("../lib/trace/tracer");
const _pipereadable = require("../pipe-readable");
const _bailouttocsr = require("../../shared/lib/lazy-dynamic/bailout-to-csr");
const _hooksservercontext = require("../../client/components/hooks-server-context");
const _isnextroutererror = require("../../client/components/is-next-router-error");
const _iserror = require("../../lib/is-error");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getDigestForWellKnownError(error) {
    // If we're bailing out to CSR, we don't need to log the error.
    if ((0, _bailouttocsr.isBailoutToCSRError)(error)) return error.digest;
    // If this is a navigation error, we don't need to log the error.
    if ((0, _isnextroutererror.isNextRouterError)(error)) return error.digest;
    // If this error occurs, we know that we should be stopping the static
    // render. This is only thrown in static generation when PPR is not enabled,
    // which causes the whole page to be marked as dynamic. We don't need to
    // tell the user about this error, as it's not actionable.
    if ((0, _hooksservercontext.isDynamicServerError)(error)) return error.digest;
    return undefined;
}
function createFlightReactServerErrorHandler(shouldFormatError, onReactServerRenderError) {
    return (thrownValue)=>{
        if (typeof thrownValue === 'string') {
            // TODO-APP: look at using webcrypto instead. Requires a promise to be awaited.
            return (0, _stringhash.default)(thrownValue).toString();
        }
        // If the response was closed, we don't need to log the error.
        if ((0, _pipereadable.isAbortError)(thrownValue)) return;
        const digest = getDigestForWellKnownError(thrownValue);
        if (digest) {
            return digest;
        }
        const err = (0, _iserror.getProperError)(thrownValue);
        // If the error already has a digest, respect the original digest,
        // so it won't get re-generated into another new error.
        if (!err.digest) {
            // TODO-APP: look at using webcrypto instead. Requires a promise to be awaited.
            err.digest = (0, _stringhash.default)(err.message + err.stack || '').toString();
        }
        // Format server errors in development to add more helpful error messages
        if (shouldFormatError) {
            (0, _formatservererror.formatServerError)(err);
        }
        // Record exception in an active span, if available.
        const span = (0, _tracer.getTracer)().getActiveScopeSpan();
        if (span) {
            span.recordException(err);
            span.setStatus({
                code: _tracer.SpanStatusCode.ERROR,
                message: err.message
            });
        }
        onReactServerRenderError(err);
        return err.digest;
    };
}
function createHTMLReactServerErrorHandler(shouldFormatError, isNextExport, reactServerErrors, silenceLogger, onReactServerRenderError) {
    return (thrownValue)=>{
        var _err_message;
        if (typeof thrownValue === 'string') {
            // TODO-APP: look at using webcrypto instead. Requires a promise to be awaited.
            return (0, _stringhash.default)(thrownValue).toString();
        }
        // If the response was closed, we don't need to log the error.
        if ((0, _pipereadable.isAbortError)(thrownValue)) return;
        const digest = getDigestForWellKnownError(thrownValue);
        if (digest) {
            return digest;
        }
        const err = (0, _iserror.getProperError)(thrownValue);
        // If the error already has a digest, respect the original digest,
        // so it won't get re-generated into another new error.
        if (!err.digest) {
            // TODO-APP: look at using webcrypto instead. Requires a promise to be awaited.
            err.digest = (0, _stringhash.default)(err.message + (err.stack || '')).toString();
        }
        // @TODO by putting this here and not at the top it is possible that
        // we don't error the build in places we actually expect to
        if (!reactServerErrors.has(err.digest)) {
            reactServerErrors.set(err.digest, err);
        }
        // Format server errors in development to add more helpful error messages
        if (shouldFormatError) {
            (0, _formatservererror.formatServerError)(err);
        }
        // Don't log the suppressed error during export
        if (!(isNextExport && (err == null ? void 0 : (_err_message = err.message) == null ? void 0 : _err_message.includes('The specific message is omitted in production builds to avoid leaking sensitive details.')))) {
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
                onReactServerRenderError == null ? void 0 : onReactServerRenderError(err);
            }
        }
        return err.digest;
    };
}
function createHTMLErrorHandler(shouldFormatError, isNextExport, reactServerErrors, allCapturedErrors, silenceLogger, onHTMLRenderSSRError) {
    return (thrownValue, errorInfo)=>{
        var _err_message;
        let isSSRError = true;
        allCapturedErrors.push(thrownValue);
        // If the response was closed, we don't need to log the error.
        if ((0, _pipereadable.isAbortError)(thrownValue)) return;
        const digest = getDigestForWellKnownError(thrownValue);
        if (digest) {
            return digest;
        }
        const err = (0, _iserror.getProperError)(thrownValue);
        // If the error already has a digest, respect the original digest,
        // so it won't get re-generated into another new error.
        if (err.digest) {
            if (reactServerErrors.has(err.digest)) {
                // This error is likely an obfuscated error from react-server.
                // We recover the original error here.
                thrownValue = reactServerErrors.get(err.digest);
                isSSRError = false;
            } else {
            // The error is not from react-server but has a digest
            // from other means so we don't need to produce a new one
            }
        } else {
            err.digest = (0, _stringhash.default)(err.message + ((errorInfo == null ? void 0 : errorInfo.componentStack) || err.stack || '')).toString();
        }
        // Format server errors in development to add more helpful error messages
        if (shouldFormatError) {
            (0, _formatservererror.formatServerError)(err);
        }
        // Don't log the suppressed error during export
        if (!(isNextExport && (err == null ? void 0 : (_err_message = err.message) == null ? void 0 : _err_message.includes('The specific message is omitted in production builds to avoid leaking sensitive details.')))) {
            // Record exception in an active span, if available.
            const span = (0, _tracer.getTracer)().getActiveScopeSpan();
            if (span) {
                span.recordException(err);
                span.setStatus({
                    code: _tracer.SpanStatusCode.ERROR,
                    message: err.message
                });
            }
            if (!silenceLogger && // HTML errors contain RSC errors as well, filter them out before reporting
            isSSRError) {
                onHTMLRenderSSRError(err, errorInfo);
            }
        }
        return err.digest;
    };
}
function isUserLandError(err) {
    return !(0, _pipereadable.isAbortError)(err) && !(0, _bailouttocsr.isBailoutToCSRError)(err) && !(0, _isnextroutererror.isNextRouterError)(err);
}

//# sourceMappingURL=create-error-handler.js.map