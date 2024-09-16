"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "wrapNormalizerFn", {
    enumerable: true,
    get: function() {
        return wrapNormalizerFn;
    }
});
function wrapNormalizerFn(fn) {
    return {
        normalize: fn
    };
}

//# sourceMappingURL=wrap-normalizer-fn.js.map