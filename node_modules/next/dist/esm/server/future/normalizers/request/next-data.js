import { denormalizePagePath } from "../../../../shared/lib/page-path/denormalize-page-path";
import { PrefixPathnameNormalizer } from "./prefix";
import { SuffixPathnameNormalizer } from "./suffix";
export class NextDataPathnameNormalizer {
    constructor(buildID){
        this.suffix = new SuffixPathnameNormalizer(".json");
        if (!buildID) {
            throw new Error("Invariant: buildID is required");
        }
        this.prefix = new PrefixPathnameNormalizer(`/_next/data/${buildID}`);
    }
    match(pathname) {
        return this.prefix.match(pathname) && this.suffix.match(pathname);
    }
    normalize(pathname, matched) {
        // If we're not matched and we don't match, we don't need to normalize.
        if (!matched && !this.match(pathname)) return pathname;
        pathname = this.prefix.normalize(pathname, true);
        pathname = this.suffix.normalize(pathname, true);
        return denormalizePagePath(pathname);
    }
}

//# sourceMappingURL=next-data.js.map