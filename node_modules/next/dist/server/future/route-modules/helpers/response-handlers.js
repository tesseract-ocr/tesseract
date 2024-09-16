"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    handleBadRequestResponse: null,
    handleInternalServerErrorResponse: null,
    handleMethodNotAllowedResponse: null,
    handleNotFoundResponse: null,
    handleRedirectResponse: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    handleBadRequestResponse: function() {
        return handleBadRequestResponse;
    },
    handleInternalServerErrorResponse: function() {
        return handleInternalServerErrorResponse;
    },
    handleMethodNotAllowedResponse: function() {
        return handleMethodNotAllowedResponse;
    },
    handleNotFoundResponse: function() {
        return handleNotFoundResponse;
    },
    handleRedirectResponse: function() {
        return handleRedirectResponse;
    }
});
const _requestcookies = require("../../../web/spec-extension/adapters/request-cookies");
function handleRedirectResponse(url, mutableCookies, status) {
    const headers = new Headers({
        location: url
    });
    (0, _requestcookies.appendMutableCookies)(headers, mutableCookies);
    return new Response(null, {
        status,
        headers
    });
}
function handleBadRequestResponse() {
    return new Response(null, {
        status: 400
    });
}
function handleNotFoundResponse() {
    return new Response(null, {
        status: 404
    });
}
function handleMethodNotAllowedResponse() {
    return new Response(null, {
        status: 405
    });
}
function handleInternalServerErrorResponse() {
    return new Response(null, {
        status: 500
    });
}

//# sourceMappingURL=response-handlers.js.map