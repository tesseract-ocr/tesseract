// The Turbopack HMR client can't be properly omitted at the moment (WEB-1589),
// so instead we remap its import to this file in webpack builds.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "connect", {
    enumerable: true,
    get: function() {
        return connect;
    }
});
function connect() {}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=noop-turbopack-hmr.js.map