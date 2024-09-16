"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ActionPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return ActionPathnameNormalizer;
    }
});
const _constants = require("../../../../lib/constants");
const _suffix = require("./suffix");
class ActionPathnameNormalizer extends _suffix.SuffixPathnameNormalizer {
    constructor(){
        super(_constants.ACTION_SUFFIX);
    }
}

//# sourceMappingURL=action.js.map