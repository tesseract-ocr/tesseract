// This file must be bundled in the app's client layer, it shouldn't be directly
// imported by the server.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    callServer: null,
    createServerReference: null,
    findSourceMapURL: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    callServer: function() {
        return _appcallserver.callServer;
    },
    createServerReference: function() {
        return createServerReference;
    },
    findSourceMapURL: function() {
        return _appfindsourcemapurl.findSourceMapURL;
    }
});
const _appcallserver = require("next/dist/client/app-call-server");
const _appfindsourcemapurl = require("next/dist/client/app-find-source-map-url");
const createServerReference = (!!process.env.NEXT_RUNTIME ? require('react-server-dom-webpack/client.edge') : require('react-server-dom-webpack/client')).createServerReference;

//# sourceMappingURL=action-client-wrapper.js.map