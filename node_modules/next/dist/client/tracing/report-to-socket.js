"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return reportToSocket;
    }
});
const _websocket = require("../components/react-dev-overlay/pages/websocket");
function reportToSocket(span) {
    if (span.state.state !== 'ended') {
        throw new Error('Expected span to be ended');
    }
    (0, _websocket.sendMessage)(JSON.stringify({
        event: 'span-end',
        startTime: span.startTime,
        endTime: span.state.endTime,
        spanName: span.name,
        attributes: span.attributes
    }));
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=report-to-socket.js.map