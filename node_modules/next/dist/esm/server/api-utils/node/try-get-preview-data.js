import { checkIsOnDemandRevalidate } from '../.';
import { clearPreviewData, COOKIE_NAME_PRERENDER_BYPASS, COOKIE_NAME_PRERENDER_DATA, SYMBOL_PREVIEW_DATA } from '../index';
import { RequestCookies } from '../../web/spec-extension/cookies';
import { HeadersAdapter } from '../../web/spec-extension/adapters/headers';
export function tryGetPreviewData(req, res, options, multiZoneDraftMode) {
    var _cookies_get, _cookies_get1;
    // if an On-Demand revalidation is being done preview mode
    // is disabled
    if (options && checkIsOnDemandRevalidate(req, options).isOnDemandRevalidate) {
        return false;
    }
    // Read cached preview data if present
    // TODO: use request metadata instead of a symbol
    if (SYMBOL_PREVIEW_DATA in req) {
        return req[SYMBOL_PREVIEW_DATA];
    }
    const headers = HeadersAdapter.from(req.headers);
    const cookies = new RequestCookies(headers);
    const previewModeId = (_cookies_get = cookies.get(COOKIE_NAME_PRERENDER_BYPASS)) == null ? void 0 : _cookies_get.value;
    const tokenPreviewData = (_cookies_get1 = cookies.get(COOKIE_NAME_PRERENDER_DATA)) == null ? void 0 : _cookies_get1.value;
    // Case: preview mode cookie set but data cookie is not set
    if (previewModeId && !tokenPreviewData && previewModeId === options.previewModeId) {
        // This is "Draft Mode" which doesn't use
        // previewData, so we return an empty object
        // for backwards compat with "Preview Mode".
        const data = {};
        Object.defineProperty(req, SYMBOL_PREVIEW_DATA, {
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
            clearPreviewData(res);
        }
        return false;
    }
    // Case: preview session is for an old build.
    if (previewModeId !== options.previewModeId) {
        if (!multiZoneDraftMode) {
            clearPreviewData(res);
        }
        return false;
    }
    let encryptedPreviewData;
    try {
        const jsonwebtoken = require('next/dist/compiled/jsonwebtoken');
        encryptedPreviewData = jsonwebtoken.verify(tokenPreviewData, options.previewModeSigningKey);
    } catch  {
        // TODO: warn
        clearPreviewData(res);
        return false;
    }
    const { decryptWithSecret } = require('../../crypto-utils');
    const decryptedPreviewData = decryptWithSecret(Buffer.from(options.previewModeEncryptionKey), encryptedPreviewData.data);
    try {
        // TODO: strict runtime type checking
        const data = JSON.parse(decryptedPreviewData);
        // Cache lookup
        Object.defineProperty(req, SYMBOL_PREVIEW_DATA, {
            value: data,
            enumerable: false
        });
        return data;
    } catch  {
        return false;
    }
}

//# sourceMappingURL=try-get-preview-data.js.map