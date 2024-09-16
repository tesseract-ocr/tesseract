import { SERVER_DIRECTORY } from "../../../../../shared/lib/constants";
import { PrefixingNormalizer } from "../../prefixing-normalizer";
export class PagesFilenameNormalizer extends PrefixingNormalizer {
    constructor(distDir){
        super(distDir, SERVER_DIRECTORY);
    }
    normalize(manifestFilename) {
        return super.normalize(manifestFilename);
    }
}

//# sourceMappingURL=pages-filename-normalizer.js.map