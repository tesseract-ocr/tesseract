"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "stringifyRequest", {
    enumerable: true,
    get: function() {
        return stringifyRequest;
    }
});
function stringifyRequest(loaderContext, request) {
    return JSON.stringify(loaderContext.utils.contextify(loaderContext.context || loaderContext.rootContext, request));
}

//# sourceMappingURL=stringify-request.js.map