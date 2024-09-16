// next-contentlayer is relying on this internal path
// https://github.com/contentlayerdev/contentlayer/blob/2f491c540e1d3667577f57fa368b150bff427aaf/packages/next-contentlayer/src/hooks/useLiveReload.ts#L1
// Drop this file if https://github.com/contentlayerdev/contentlayer/pull/649 is merged/released
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "addMessageListener", {
    enumerable: true,
    get: function() {
        return _websocket.addMessageListener;
    }
});
const _websocket = require("../../components/react-dev-overlay/pages/websocket");

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=websocket.js.map