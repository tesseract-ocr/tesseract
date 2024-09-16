"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "denormalizeAppPagePath", {
    enumerable: true,
    get: function() {
        return denormalizeAppPagePath;
    }
});
function denormalizeAppPagePath(page) {
    // `/` is normalized to `/index`
    if (page === "/index") {
        return "/";
    }
    return page;
}

//# sourceMappingURL=denormalize-app-path.js.map