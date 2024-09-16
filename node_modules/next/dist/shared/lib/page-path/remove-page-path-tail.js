"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "removePagePathTail", {
    enumerable: true,
    get: function() {
        return removePagePathTail;
    }
});
const _normalizepathsep = require("./normalize-path-sep");
function removePagePathTail(pagePath, options) {
    pagePath = (0, _normalizepathsep.normalizePathSep)(pagePath).replace(new RegExp("\\.+(?:" + options.extensions.join("|") + ")$"), "");
    if (options.keepIndex !== true) {
        pagePath = pagePath.replace(/\/index$/, "") || "/";
    }
    return pagePath;
}

//# sourceMappingURL=remove-page-path-tail.js.map