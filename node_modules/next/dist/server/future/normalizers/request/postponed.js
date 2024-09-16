"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PostponedPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return PostponedPathnameNormalizer;
    }
});
const _denormalizepagepath = require("../../../../shared/lib/page-path/denormalize-page-path");
const _prefix = require("./prefix");
const prefix = "/_next/postponed/resume";
class PostponedPathnameNormalizer extends _prefix.PrefixPathnameNormalizer {
    constructor(){
        super(prefix);
    }
    normalize(pathname, matched) {
        // If we're not matched and we don't match, we don't need to normalize.
        if (!matched && !this.match(pathname)) return pathname;
        // Remove the prefix.
        pathname = super.normalize(pathname, true);
        return (0, _denormalizepagepath.denormalizePagePath)(pathname);
    }
}

//# sourceMappingURL=postponed.js.map