"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "NextDataPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return NextDataPathnameNormalizer;
    }
});
const _denormalizepagepath = require("../../../../shared/lib/page-path/denormalize-page-path");
const _prefix = require("./prefix");
const _suffix = require("./suffix");
class NextDataPathnameNormalizer {
    constructor(buildID){
        this.suffix = new _suffix.SuffixPathnameNormalizer(".json");
        if (!buildID) {
            throw new Error("Invariant: buildID is required");
        }
        this.prefix = new _prefix.PrefixPathnameNormalizer(`/_next/data/${buildID}`);
    }
    match(pathname) {
        return this.prefix.match(pathname) && this.suffix.match(pathname);
    }
    normalize(pathname, matched) {
        // If we're not matched and we don't match, we don't need to normalize.
        if (!matched && !this.match(pathname)) return pathname;
        pathname = this.prefix.normalize(pathname, true);
        pathname = this.suffix.normalize(pathname, true);
        return (0, _denormalizepagepath.denormalizePagePath)(pathname);
    }
}

//# sourceMappingURL=next-data.js.map