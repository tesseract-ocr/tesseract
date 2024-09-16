"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCookieParser", {
    enumerable: true,
    get: function() {
        return getCookieParser;
    }
});
function getCookieParser(headers) {
    return function parseCookie() {
        const { cookie } = headers;
        if (!cookie) {
            return {};
        }
        const { parse: parseCookieFn } = require("next/dist/compiled/cookie");
        return parseCookieFn(Array.isArray(cookie) ? cookie.join("; ") : cookie);
    };
}

//# sourceMappingURL=get-cookie-parser.js.map