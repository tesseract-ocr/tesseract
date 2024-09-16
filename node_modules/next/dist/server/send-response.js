"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "sendResponse", {
    enumerable: true,
    get: function() {
        return sendResponse;
    }
});
const _pipereadable = require("./pipe-readable");
const _utils = require("./web/utils");
async function sendResponse(req, res, response, waitUntil) {
    // Don't use in edge runtime
    if (process.env.NEXT_RUNTIME !== "edge") {
        var // Copy over the response headers.
        _response_headers;
        // Copy over the response status.
        res.statusCode = response.status;
        res.statusMessage = response.statusText;
        (_response_headers = response.headers) == null ? void 0 : _response_headers.forEach((value, name)=>{
            // The append handling is special cased for `set-cookie`.
            if (name.toLowerCase() === "set-cookie") {
                // TODO: (wyattjoh) replace with native response iteration when we can upgrade undici
                for (const cookie of (0, _utils.splitCookiesString)(value)){
                    res.appendHeader(name, cookie);
                }
            } else {
                res.appendHeader(name, value);
            }
        });
        /**
     * The response can't be directly piped to the underlying response. The
     * following is duplicated from the edge runtime handler.
     *
     * See packages/next/server/next-server.ts
     */ const originalResponse = res.originalResponse;
        // A response body must not be sent for HEAD requests. See https://httpwg.org/specs/rfc9110.html#HEAD
        if (response.body && req.method !== "HEAD") {
            await (0, _pipereadable.pipeToNodeResponse)(response.body, originalResponse, waitUntil);
        } else {
            originalResponse.end();
        }
    }
}

//# sourceMappingURL=send-response.js.map