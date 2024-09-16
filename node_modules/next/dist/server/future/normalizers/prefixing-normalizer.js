"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PrefixingNormalizer", {
    enumerable: true,
    get: function() {
        return PrefixingNormalizer;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/isomorphic/path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
class PrefixingNormalizer {
    constructor(...prefixes){
        this.prefix = _path.default.posix.join(...prefixes);
    }
    normalize(pathname) {
        return _path.default.posix.join(this.prefix, pathname);
    }
}

//# sourceMappingURL=prefixing-normalizer.js.map