import path from "../../../shared/lib/isomorphic/path";
export class PrefixingNormalizer {
    constructor(...prefixes){
        this.prefix = path.posix.join(...prefixes);
    }
    normalize(pathname) {
        return path.posix.join(this.prefix, pathname);
    }
}

//# sourceMappingURL=prefixing-normalizer.js.map