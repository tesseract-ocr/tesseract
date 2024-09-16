"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    deserializeErr: null,
    invokeIpcMethod: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    deserializeErr: function() {
        return deserializeErr;
    },
    invokeIpcMethod: function() {
        return invokeIpcMethod;
    }
});
const _errorsource = require("../../../shared/lib/error-source");
const _utils = require("../../../shared/lib/utils");
const _invokerequest = require("./invoke-request");
const deserializeErr = (serializedErr)=>{
    if (!serializedErr || typeof serializedErr !== "object" || !serializedErr.stack) {
        return serializedErr;
    }
    let ErrorType = Error;
    if (serializedErr.name === "PageNotFoundError") {
        ErrorType = _utils.PageNotFoundError;
    }
    const err = new ErrorType(serializedErr.message);
    err.stack = serializedErr.stack;
    err.name = serializedErr.name;
    err.digest = serializedErr.digest;
    if (process.env.NODE_ENV === "development" && process.env.NEXT_RUNTIME !== "edge") {
        (0, _errorsource.decorateServerError)(err, serializedErr.source || "server");
    }
    return err;
};
async function invokeIpcMethod({ fetchHostname = "localhost", method, args, ipcPort, ipcKey }) {
    if (ipcPort) {
        const res = await (0, _invokerequest.invokeRequest)(`http://${fetchHostname}:${ipcPort}?key=${ipcKey}&method=${method}&args=${encodeURIComponent(JSON.stringify(args))}`, {
            method: "GET",
            headers: {}
        });
        const body = await res.text();
        if (body.startsWith("{") && body.endsWith("}")) {
            const parsedBody = JSON.parse(body);
            if (parsedBody && typeof parsedBody === "object" && "err" in parsedBody && "stack" in parsedBody.err) {
                throw deserializeErr(parsedBody.err);
            }
            return parsedBody;
        }
    }
}

//# sourceMappingURL=request-utils.js.map