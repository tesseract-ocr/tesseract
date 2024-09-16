"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isNavigationSignalError", {
    enumerable: true,
    get: function() {
        return isNavigationSignalError;
    }
});
const _notfound = require("../../client/components/not-found");
const _redirect = require("../../client/components/redirect");
const isNavigationSignalError = (err)=>(0, _notfound.isNotFoundError)(err) || (0, _redirect.isRedirectError)(err);

//# sourceMappingURL=is-navigation-signal-error.js.map