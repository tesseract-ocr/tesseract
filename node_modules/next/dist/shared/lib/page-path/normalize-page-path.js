"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "normalizePagePath", {
    enumerable: true,
    get: function() {
        return normalizePagePath;
    }
});
const _ensureleadingslash = require("./ensure-leading-slash");
const _utils = require("../router/utils");
const _utils1 = require("../utils");
function normalizePagePath(page) {
    const normalized = /^\/index(\/|$)/.test(page) && !(0, _utils.isDynamicRoute)(page) ? "/index" + page : page === '/' ? '/index' : (0, _ensureleadingslash.ensureLeadingSlash)(page);
    if (process.env.NEXT_RUNTIME !== 'edge') {
        const { posix } = require('path');
        const resolvedPage = posix.normalize(normalized);
        if (resolvedPage !== normalized) {
            throw new _utils1.NormalizeError("Requested and resolved page mismatch: " + normalized + " " + resolvedPage);
        }
    }
    return normalized;
}

//# sourceMappingURL=normalize-page-path.js.map