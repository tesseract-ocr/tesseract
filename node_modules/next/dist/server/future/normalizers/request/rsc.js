"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "RSCPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return RSCPathnameNormalizer;
    }
});
const _constants = require("../../../../lib/constants");
const _suffix = require("./suffix");
class RSCPathnameNormalizer extends _suffix.SuffixPathnameNormalizer {
    constructor(){
        super(_constants.RSC_SUFFIX);
    }
}

//# sourceMappingURL=rsc.js.map