"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parsedUrlQueryToParams", {
    enumerable: true,
    get: function() {
        return parsedUrlQueryToParams;
    }
});
function parsedUrlQueryToParams(query) {
    const params = {};
    for (const [key, value] of Object.entries(query)){
        if (typeof value === "undefined") continue;
        params[key] = value;
    }
    return params;
}

//# sourceMappingURL=parsed-url-query-to-params.js.map