"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "requestAsyncStorage", {
    enumerable: true,
    get: function() {
        return requestAsyncStorage;
    }
});
const _asynclocalstorage = require("./async-local-storage");
const requestAsyncStorage = (0, _asynclocalstorage.createAsyncLocalStorage)();

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=request-async-storage-instance.js.map