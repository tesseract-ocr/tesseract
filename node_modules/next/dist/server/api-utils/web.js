// Buffer.byteLength polyfill in the Edge runtime, with only utf8 strings
// supported at the moment.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "byteLength", {
    enumerable: true,
    get: function() {
        return byteLength;
    }
});
function byteLength(payload) {
    return new TextEncoder().encode(payload).buffer.byteLength;
}

//# sourceMappingURL=web.js.map