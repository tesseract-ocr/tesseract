import { PrefixPathnameNormalizer } from "./prefix";
export class BasePathPathnameNormalizer extends PrefixPathnameNormalizer {
    constructor(basePath){
        if (!basePath || basePath === "/") {
            throw new Error('Invariant: basePath must be set and cannot be "/"');
        }
        super(basePath);
    }
}

//# sourceMappingURL=base-path.js.map