"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "resolveHandlerError", {
    enumerable: true,
    get: function() {
        return resolveHandlerError;
    }
});
const _notfound = require("../../../../../client/components/not-found");
const _redirect = require("../../../../../client/components/redirect");
const _responsehandlers = require("../../helpers/response-handlers");
function resolveHandlerError(err) {
    if ((0, _redirect.isRedirectError)(err)) {
        const redirect = (0, _redirect.getURLFromRedirectError)(err);
        if (!redirect) {
            throw new Error("Invariant: Unexpected redirect url format");
        }
        const status = (0, _redirect.getRedirectStatusCodeFromError)(err);
        // This is a redirect error! Send the redirect response.
        return (0, _responsehandlers.handleRedirectResponse)(redirect, err.mutableCookies, status);
    }
    if ((0, _notfound.isNotFoundError)(err)) {
        // This is a not found error! Send the not found response.
        return (0, _responsehandlers.handleNotFoundResponse)();
    }
    // Return false to indicate that this is not a handled error.
    return false;
}

//# sourceMappingURL=resolve-handler-error.js.map