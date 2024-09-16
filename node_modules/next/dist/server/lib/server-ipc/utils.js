"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    actionsForbiddenHeaders: null,
    filterReqHeaders: null,
    ipcForbiddenHeaders: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    actionsForbiddenHeaders: function() {
        return actionsForbiddenHeaders;
    },
    filterReqHeaders: function() {
        return filterReqHeaders;
    },
    ipcForbiddenHeaders: function() {
        return ipcForbiddenHeaders;
    }
});
const ipcForbiddenHeaders = [
    "accept-encoding",
    "keepalive",
    "keep-alive",
    "content-encoding",
    "transfer-encoding",
    // https://github.com/nodejs/undici/issues/1470
    "connection",
    // marked as unsupported by undici: https://github.com/nodejs/undici/blob/c83b084879fa0bb8e0469d31ec61428ac68160d5/lib/core/request.js#L354
    "expect"
];
const actionsForbiddenHeaders = [
    ...ipcForbiddenHeaders,
    "content-length",
    "set-cookie"
];
const filterReqHeaders = (headers, forbiddenHeaders)=>{
    // Some browsers are not matching spec and sending Content-Length: 0. This causes issues in undici
    // https://github.com/nodejs/undici/issues/2046
    if (headers["content-length"] && headers["content-length"] === "0") {
        delete headers["content-length"];
    }
    for (const [key, value] of Object.entries(headers)){
        if (forbiddenHeaders.includes(key) || !(Array.isArray(value) || typeof value === "string")) {
            delete headers[key];
        }
    }
    return headers;
};

//# sourceMappingURL=utils.js.map