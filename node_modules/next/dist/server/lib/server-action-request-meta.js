"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getIsServerAction: null,
    getServerActionRequestMetadata: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getIsServerAction: function() {
        return getIsServerAction;
    },
    getServerActionRequestMetadata: function() {
        return getServerActionRequestMetadata;
    }
});
const _approuterheaders = require("../../client/components/app-router-headers");
function getServerActionRequestMetadata(req) {
    let actionId;
    let contentType;
    if (req.headers instanceof Headers) {
        actionId = req.headers.get(_approuterheaders.ACTION.toLowerCase()) ?? null;
        contentType = req.headers.get("content-type");
    } else {
        actionId = req.headers[_approuterheaders.ACTION.toLowerCase()] ?? null;
        contentType = req.headers["content-type"] ?? null;
    }
    const isURLEncodedAction = Boolean(req.method === "POST" && contentType === "application/x-www-form-urlencoded");
    const isMultipartAction = Boolean(req.method === "POST" && (contentType == null ? void 0 : contentType.startsWith("multipart/form-data")));
    const isFetchAction = Boolean(actionId !== undefined && typeof actionId === "string" && req.method === "POST");
    const isServerAction = Boolean(isFetchAction || isURLEncodedAction || isMultipartAction);
    return {
        actionId,
        isURLEncodedAction,
        isMultipartAction,
        isFetchAction,
        isServerAction
    };
}
function getIsServerAction(req) {
    return getServerActionRequestMetadata(req).isServerAction;
}

//# sourceMappingURL=server-action-request-meta.js.map