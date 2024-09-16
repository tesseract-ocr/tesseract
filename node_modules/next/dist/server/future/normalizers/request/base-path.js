"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "BasePathPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return BasePathPathnameNormalizer;
    }
});
const _prefix = require("./prefix");
class BasePathPathnameNormalizer extends _prefix.PrefixPathnameNormalizer {
    constructor(basePath){
        if (!basePath || basePath === "/") {
            throw new Error('Invariant: basePath must be set and cannot be "/"');
        }
        super(basePath);
    }
}

//# sourceMappingURL=base-path.js.map