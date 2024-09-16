import { PAGE_TYPES } from "../../../../../lib/page-types";
import { AbsoluteFilenameNormalizer } from "../../absolute-filename-normalizer";
/**
 * DevAppPageNormalizer is a normalizer that is used to normalize a pathname
 * to a page in the `app` directory.
 */ export class DevAppPageNormalizer extends AbsoluteFilenameNormalizer {
    constructor(appDir, extensions){
        super(appDir, extensions, PAGE_TYPES.APP);
    }
}

//# sourceMappingURL=app-page-normalizer.js.map