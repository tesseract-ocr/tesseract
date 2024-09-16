/**
 * Normalizers combines many normalizers into a single normalizer interface that
 * will normalize the inputted pathname with each normalizer in order.
 */ export class Normalizers {
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