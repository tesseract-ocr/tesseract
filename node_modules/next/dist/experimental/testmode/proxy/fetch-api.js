"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "handleFetch", {
    enumerable: true,
    get: function() {
        return handleFetch;
    }
});
const _types = require("./types");
function buildRequest(req) {
    const { request: proxyRequest } = req;
    const { url, headers, body, ...options } = proxyRequest;
    return new Request(url, {
        ...options,
        headers: new Headers(headers),
        body: body ? Buffer.from(body, 'base64') : null
    });
}
async function buildResponse(response) {
    if (!response) {
        return _types.UNHANDLED;
    }
    if (response === 'abort') {
        return _types.ABORT;
    }
    if (response === 'continue') {
        return _types.CONTINUE;
    }
    const { status, headers, body } = response;
    return {
        api: 'fetch',
        response: {
            status,
            headers: Array.from(headers),
            body: body ? Buffer.from(await response.arrayBuffer()).toString('base64') : null
        }
    };
}
async function handleFetch(req, onFetch) {
    const { testData } = req;
    const request = buildRequest(req);
    const response = await onFetch(testData, request);
    return buildResponse(response);
}

//# sourceMappingURL=fetch-api.js.map