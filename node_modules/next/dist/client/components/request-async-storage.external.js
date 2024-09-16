"TURBOPACK { transition: next-shared }";
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getExpectedRequestStore: null,
    requestAsyncStorage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getExpectedRequestStore: function() {
        return getExpectedRequestStore;
    },
    requestAsyncStorage: function() {
        return _requestasyncstorageinstance.requestAsyncStorage;
    }
});
const _requestasyncstorageinstance = require("./request-async-storage-instance");
function getExpectedRequestStore(callingExpression) {
    const store = _requestasyncstorageinstance.requestAsyncStorage.getStore();
    if (store) return store;
    throw new Error("`" + callingExpression + "` was called outside a request scope. Read more: https://nextjs.org/docs/messages/next-dynamic-api-wrong-context");
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=request-async-storage.external.js.map