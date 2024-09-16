"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PagesFilenameNormalizer", {
    enumerable: true,
    get: function() {
        return PagesFilenameNormalizer;
    }
});
const _constants = require("../../../../../shared/lib/constants");
const _prefixingnormalizer = require("../../prefixing-normalizer");
class PagesFilenameNormalizer extends _prefixingnormalizer.PrefixingNormalizer {
    constructor(distDir){
        super(distDir, _constants.SERVER_DIRECTORY);
    }
    normalize(manifestFilename) {
        return super.normalize(manifestFilename);
    }
}

//# sourceMappingURL=pages-filename-normalizer.js.map