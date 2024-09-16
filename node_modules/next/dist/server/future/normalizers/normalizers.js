"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "Normalizers", {
    enumerable: true,
    get: function() {
        return Normalizers;
    }
});
class Normalizers {
    constructor(normalizers = []){
        this.normalizers = normalizers;
    }
    push(normalizer) {
        this.normalizers.push(normalizer);
    }
    normalize(pathname) {
        return this.normalizers.reduce((normalized, normalizer)=>normalizer.normalize(normalized), pathname);
    }
}

//# sourceMappingURL=normalizers.js.map