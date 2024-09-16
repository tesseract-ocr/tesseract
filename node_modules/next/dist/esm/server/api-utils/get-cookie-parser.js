/**
 * Parse cookies from the `headers` of request
 * @param req request object
 */ export function getCookieParser(headers) {
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