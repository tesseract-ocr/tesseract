"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "tryGetPreviewData", {
    enumerable: true,
    get: function() {
        return tryGetPreviewData;
    }
});
const _ = require("../.");
const _index = require("../index");
const _cookies = require("../../web/spec-extension/cookies");
const _headers = require("../../web/spec-extension/adapters/headers");
function tryGetPreviewData(req, res, options, multiZoneDraftMode) {
    var _cookies_get, _cookies_get1;
    // if an On-Demand revalidation is being done preview mode
    // is disabled
    if (options && (0, _.checkIsOnDemandRevalidate)(req, options).isOnDemandRevalidate) {
        return false;
    }
    // Read cached preview data if present
    // TODO: use request metadata instead of a symbol
    if (_index.SYMBOL_PREVIEW_DATA in req) {
        return req[_index.SYMBOL_PREVIEW_DATA];
    }
    const headers = _headers.HeadersAdapter.from(req.headers);
    const cookies = new _cookies.RequestCookies(headers);
    const previewModeId = (_cookies_get = cookies.get(_index.COOKIE_NAME_PRERENDER_BYPASS)) == null ? void 0 : _cookies_get.value;
    const tokenPreviewData = (_cookies_get1 = cookies.get(_index.COOKIE_NAME_PRERENDER_DATA)) == null ? void 0 : _cookies_get1.value;
    // Case: preview mode cookie set but data cookie is not set
    if (previewModeId && !tokenPreviewData && previewModeId === options.previewModeId) {
        // This is "Draft Mode" which doesn't use
        // previewData, so we return an empty object
        // for backwards compat with "Preview Mode".
        const data = {};
        Object.defineProperty(req, _index.SYMBOL_PREVIEW_DATA, {
            value: data,
            enumerable: false
        });
        return data;
    }
    // Case: neither cookie is set.
    if (!previewModeId && !tokenPreviewData) {
        return false;
    }
    // Case: one cookie is set, but not the other.
    if (!previewModeId || !tokenPreviewData) {
        if (!multiZoneDraftMode) {
            (0, _index.clearPreviewData)(res);
        }
        return false;
    }
    // Case: preview session is for an old build.
    if (previewModeId !== options.previewModeId) {
        if (!multiZoneDraftMode) {
            (0, _index.clearPreviewData)(res);
        }
        return false;
    }
    let encryptedPreviewData;
    try {
        const jsonwebtoken = require("next/dist/compiled/jsonwebtoken");
        encryptedPreviewData = jsonwebtoken.verify(tokenPreviewData, options.previewModeSigningKey);
    } catch  {
        // TODO: warn
        (0, _index.clearPreviewData)(res);
        return false;
    }
    const { decryptWithSecret } = require("../../crypto-utils");
    const decryptedPreviewData = decryptWithSecret(Buffer.from(options.previewModeEncryptionKey), encryptedPreviewData.data);
    try {
        // TODO: strict runtime type checking
        const data = JSON.parse(decryptedPreviewData);
        // Cache lookup
        Object.defineProperty(req, _index.SYMBOL_PREVIEW_DATA, {
            value: data,
            enumerable: false
        });
        return data;
    } catch  {
        return false;
    }
}

//# sourceMappingURL=try-get-preview-data.js.map