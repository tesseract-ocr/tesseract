"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    fromResponseCacheEntry: null,
    toResponseCacheEntry: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    fromResponseCacheEntry: function() {
        return fromResponseCacheEntry;
    },
    toResponseCacheEntry: function() {
        return toResponseCacheEntry;
    }
});
const _renderresult = /*#__PURE__*/ _interop_require_default(require("../render-result"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function fromResponseCacheEntry(cacheEntry) {
    var _cacheEntry_value;
    return {
        ...cacheEntry,
        value: ((_cacheEntry_value = cacheEntry.value) == null ? void 0 : _cacheEntry_value.kind) === "PAGE" ? {
            kind: "PAGE",
            html: await cacheEntry.value.html.toUnchunkedString(true),
            postponed: cacheEntry.value.postponed,
            pageData: cacheEntry.value.pageData,
            headers: cacheEntry.value.headers,
            status: cacheEntry.value.status
        } : cacheEntry.value
    };
}
async function toResponseCacheEntry(response) {
    var _response_value, _response_value1;
    if (!response) return null;
    if (((_response_value = response.value) == null ? void 0 : _response_value.kind) === "FETCH") {
        throw new Error("Invariant: unexpected cachedResponse of kind fetch in response cache");
    }
    return {
        isMiss: response.isMiss,
        isStale: response.isStale,
        revalidate: response.revalidate,
        value: ((_response_value1 = response.value) == null ? void 0 : _response_value1.kind) === "PAGE" ? {
            kind: "PAGE",
            html: _renderresult.default.fromStatic(response.value.html),
            pageData: response.value.pageData,
            postponed: response.value.postponed,
            headers: response.value.headers,
            status: response.value.status
        } : response.value
    };
}

//# sourceMappingURL=utils.js.map