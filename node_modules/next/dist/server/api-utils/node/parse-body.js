"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parseBody", {
    enumerable: true,
    get: function() {
        return parseBody;
    }
});
const _contenttype = require("next/dist/compiled/content-type");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../../lib/is-error"));
const _index = require("../index");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
/**
 * Parse `JSON` and handles invalid `JSON` strings
 * @param str `JSON` string
 */ function parseJson(str) {
    if (str.length === 0) {
        // special-case empty json body, as it's a common client-side mistake
        return {};
    }
    try {
        return JSON.parse(str);
    } catch (e) {
        throw new _index.ApiError(400, "Invalid JSON");
    }
}
async function parseBody(req, limit) {
    let contentType;
    try {
        contentType = (0, _contenttype.parse)(req.headers["content-type"] || "text/plain");
    } catch  {
        contentType = (0, _contenttype.parse)("text/plain");
    }
    const { type, parameters } = contentType;
    const encoding = parameters.charset || "utf-8";
    let buffer;
    try {
        const getRawBody = require("next/dist/compiled/raw-body");
        buffer = await getRawBody(req, {
            encoding,
            limit
        });
    } catch (e) {
        if ((0, _iserror.default)(e) && e.type === "entity.too.large") {
            throw new _index.ApiError(413, `Body exceeded ${limit} limit`);
        } else {
            throw new _index.ApiError(400, "Invalid body");
        }
    }
    const body = buffer.toString();
    if (type === "application/json" || type === "application/ld+json") {
        return parseJson(body);
    } else if (type === "application/x-www-form-urlencoded") {
        const qs = require("querystring");
        return qs.decode(body);
    } else {
        return body;
    }
}

//# sourceMappingURL=parse-body.js.map