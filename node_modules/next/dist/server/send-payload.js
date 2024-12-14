"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    sendEtagResponse: null,
    sendRenderResult: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    sendEtagResponse: function() {
        return sendEtagResponse;
    },
    sendRenderResult: function() {
        return sendRenderResult;
    }
});
const _utils = require("../shared/lib/utils");
const _etag = require("./lib/etag");
const _fresh = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/fresh"));
const _revalidate = require("./lib/revalidate");
const _approuterheaders = require("../client/components/app-router-headers");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function sendEtagResponse(req, res, etag) {
    if (etag) {
        /**
     * The server generating a 304 response MUST generate any of the
     * following header fields that would have been sent in a 200 (OK)
     * response to the same request: Cache-Control, Content-Location, Date,
     * ETag, Expires, and Vary. https://tools.ietf.org/html/rfc7232#section-4.1
     */ res.setHeader('ETag', etag);
    }
    if ((0, _fresh.default)(req.headers, {
        etag
    })) {
        res.statusCode = 304;
        res.end();
        return true;
    }
    return false;
}
async function sendRenderResult({ req, res, result, type, generateEtags, poweredByHeader, revalidate, expireTime }) {
    if ((0, _utils.isResSent)(res)) {
        return;
    }
    if (poweredByHeader && type === 'html') {
        res.setHeader('X-Powered-By', 'Next.js');
    }
    // If cache control is already set on the response we don't
    // override it to allow users to customize it via next.config
    if (typeof revalidate !== 'undefined' && !res.getHeader('Cache-Control')) {
        res.setHeader('Cache-Control', (0, _revalidate.formatRevalidate)({
            revalidate,
            expireTime
        }));
    }
    const payload = result.isDynamic ? null : result.toUnchunkedString();
    if (generateEtags && payload !== null) {
        const etag = (0, _etag.generateETag)(payload);
        if (sendEtagResponse(req, res, etag)) {
            return;
        }
    }
    if (!res.getHeader('Content-Type')) {
        res.setHeader('Content-Type', result.contentType ? result.contentType : type === 'rsc' ? _approuterheaders.RSC_CONTENT_TYPE_HEADER : type === 'json' ? 'application/json' : 'text/html; charset=utf-8');
    }
    if (payload) {
        res.setHeader('Content-Length', Buffer.byteLength(payload));
    }
    if (req.method === 'HEAD') {
        res.end(null);
        return;
    }
    if (payload !== null) {
        res.end(payload);
        return;
    }
    // Pipe the render result to the response after we get a writer for it.
    await result.pipeToNodeResponse(res);
}

//# sourceMappingURL=send-payload.js.map