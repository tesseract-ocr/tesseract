"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    cookies: null,
    draftMode: null,
    headers: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    cookies: function() {
        return cookies;
    },
    draftMode: function() {
        return draftMode;
    },
    headers: function() {
        return headers;
    }
});
const _requestcookies = require("../../server/web/spec-extension/adapters/request-cookies");
const _headers = require("../../server/web/spec-extension/adapters/headers");
const _cookies = require("../../server/web/spec-extension/cookies");
const _actionasyncstorageexternal = require("./action-async-storage.external");
const _draftmode = require("./draft-mode");
const _dynamicrendering = require("../../server/app-render/dynamic-rendering");
const _staticgenerationasyncstorageexternal = require("./static-generation-async-storage.external");
const _requestasyncstorageexternal = require("./request-async-storage.external");
function headers() {
    const callingExpression = "headers";
    const staticGenerationStore = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
    if (staticGenerationStore) {
        if (staticGenerationStore.forceStatic) {
            // When we are forcing static we don't mark this as a Dynamic read and we return an empty headers object
            return _headers.HeadersAdapter.seal(new Headers({}));
        } else {
            // We will return a real headers object below so we mark this call as reading from a dynamic data source
            (0, _dynamicrendering.trackDynamicDataAccessed)(staticGenerationStore, callingExpression);
        }
    }
    return (0, _requestasyncstorageexternal.getExpectedRequestStore)(callingExpression).headers;
}
function cookies() {
    const callingExpression = "cookies";
    const staticGenerationStore = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
    if (staticGenerationStore) {
        if (staticGenerationStore.forceStatic) {
            // When we are forcing static we don't mark this as a Dynamic read and we return an empty cookies object
            return _requestcookies.RequestCookiesAdapter.seal(new _cookies.RequestCookies(new Headers({})));
        } else {
            // We will return a real headers object below so we mark this call as reading from a dynamic data source
            (0, _dynamicrendering.trackDynamicDataAccessed)(staticGenerationStore, callingExpression);
        }
    }
    const requestStore = (0, _requestasyncstorageexternal.getExpectedRequestStore)(callingExpression);
    const asyncActionStore = _actionasyncstorageexternal.actionAsyncStorage.getStore();
    if ((asyncActionStore == null ? void 0 : asyncActionStore.isAction) || (asyncActionStore == null ? void 0 : asyncActionStore.isAppRoute)) {
        // We can't conditionally return different types here based on the context.
        // To avoid confusion, we always return the readonly type here.
        return requestStore.mutableCookies;
    }
    return requestStore.cookies;
}
function draftMode() {
    const callingExpression = "draftMode";
    const requestStore = (0, _requestasyncstorageexternal.getExpectedRequestStore)(callingExpression);
    return new _draftmode.DraftMode(requestStore.draftMode);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=headers.js.map