// This file should be opted into the react-server layer
// eslint-disable-next-line import/no-extraneous-dependencies
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    decodeAction: null,
    decodeFormState: null,
    decodeReply: null,
    decodeReplyFromBusboy: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    decodeAction: function() {
        return _servernode.decodeAction;
    },
    decodeFormState: function() {
        return _servernode.decodeFormState;
    },
    decodeReply: function() {
        return _servernode.decodeReply;
    },
    decodeReplyFromBusboy: function() {
        return _servernode.decodeReplyFromBusboy;
    }
});
const _servernode = require("react-server-dom-webpack/server.node");

//# sourceMappingURL=react-server.node.js.map