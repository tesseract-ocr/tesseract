/**
 * Cleans a URL by stripping the protocol, host, and search params.
 *
 * @param urlString the url to clean
 * @returns the cleaned url
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "cleanURL", {
    enumerable: true,
    get: function() {
        return cleanURL;
    }
});
function cleanURL(url) {
    const u = new URL(url);
    u.host = "localhost:3000";
    u.search = "";
    u.protocol = "http";
    return u;
}

//# sourceMappingURL=clean-url.js.map