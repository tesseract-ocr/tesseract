"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "validateURL", {
    enumerable: true,
    get: function() {
        return validateURL;
    }
});
const DUMMY_ORIGIN = "http://n";
const INVALID_URL_MESSAGE = "Invalid request URL";
function validateURL(url) {
    if (!url) {
        throw new Error(INVALID_URL_MESSAGE);
    }
    try {
        const parsed = new URL(url, DUMMY_ORIGIN);
        // Avoid origin change by extra slashes in pathname
        if (parsed.origin !== DUMMY_ORIGIN) {
            throw new Error(INVALID_URL_MESSAGE);
        }
        return url;
    } catch  {
        throw new Error(INVALID_URL_MESSAGE);
    }
}

//# sourceMappingURL=validate-url.js.map