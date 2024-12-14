"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "matchNextDataPathname", {
    enumerable: true,
    get: function() {
        return matchNextDataPathname;
    }
});
const _pathmatch = require("../../shared/lib/router/utils/path-match");
const matcher = (0, _pathmatch.getPathMatch)('/_next/data/:path*');
function matchNextDataPathname(pathname) {
    if (typeof pathname !== 'string') return false;
    return matcher(pathname);
}

//# sourceMappingURL=match-next-data-pathname.js.map