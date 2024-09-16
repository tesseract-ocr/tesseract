"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    allowedStatusCodes: null,
    getRedirectStatus: null,
    modifyRouteRegex: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    allowedStatusCodes: function() {
        return allowedStatusCodes;
    },
    getRedirectStatus: function() {
        return getRedirectStatus;
    },
    modifyRouteRegex: function() {
        return modifyRouteRegex;
    }
});
const _redirectstatuscode = require("../client/components/redirect-status-code");
const allowedStatusCodes = new Set([
    301,
    302,
    303,
    307,
    308
]);
function getRedirectStatus(route) {
    return route.statusCode || (route.permanent ? _redirectstatuscode.RedirectStatusCode.PermanentRedirect : _redirectstatuscode.RedirectStatusCode.TemporaryRedirect);
}
function modifyRouteRegex(regex, restrictedPaths) {
    if (restrictedPaths) {
        regex = regex.replace(/\^/, `^(?!${restrictedPaths.map((path)=>path.replace(/\//g, "\\/")).join("|")})`);
    }
    regex = regex.replace(/\$$/, "(?:\\/)?$");
    return regex;
}

//# sourceMappingURL=redirect-status.js.map