export class SuffixPathnameNormalizer {
    constructor(suffix){
        this.suffix = suffix;
    }
    match(pathname) {
        // If the pathname doesn't end in the suffix, we don't match.
        if (!pathname.endsWith(this.suffix)) return false;
        return true;
    }
    normalize(pathname, matched) {
        // If we're not matched and we don't match, we don't need to normalize.
        if (!matched && !this.match(pathname)) return pathname;
        return pathname.substring(0, pathname.length - this.suffix.length);
    }
}

//# sourceMappingURL=suffix.js.map