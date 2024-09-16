"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "UnderscoreNormalizer", {
    enumerable: true,
    get: function() {
        return UnderscoreNormalizer;
    }
});
class UnderscoreNormalizer {
    normalize(pathname) {
        return pathname.replace(/%5F/g, "_");
    }
}

//# sourceMappingURL=underscore-normalizer.js.map