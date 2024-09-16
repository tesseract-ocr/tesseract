"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PrefixPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return PrefixPathnameNormalizer;
    }
});
class PrefixPathnameNormalizer {
    constructor(prefix){
        this.prefix = prefix;
        if (prefix.endsWith("/")) {
            throw new Error(`PrefixPathnameNormalizer: prefix "${prefix}" should not end with a slash`);
        }
    }
    match(pathname) {
        // If the pathname doesn't start with the prefix, we don't match.
        if (pathname !== this.prefix && !pathname.startsWith(this.prefix + "/")) {
            return false;
        }
        return true;
    }
    normalize(pathname, matched) {
        // If we're not matched and we don't match, we don't need to normalize.
        if (!matched && !this.match(pathname)) return pathname;
        if (pathname.length === this.prefix.length) {
            return "/";
        }
        return pathname.substring(this.prefix.length);
    }
}

//# sourceMappingURL=prefix.js.map