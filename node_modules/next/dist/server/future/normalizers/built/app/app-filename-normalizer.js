"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AppFilenameNormalizer", {
    enumerable: true,
    get: function() {
        return AppFilenameNormalizer;
    }
});
const _constants = require("../../../../../shared/lib/constants");
const _prefixingnormalizer = require("../../prefixing-normalizer");
class AppFilenameNormalizer extends _prefixingnormalizer.PrefixingNormalizer {
    constructor(distDir){
        super(distDir, _constants.SERVER_DIRECTORY);
    }
    normalize(manifestFilename) {
        return super.normalize(manifestFilename);
    }
}

//# sourceMappingURL=app-filename-normalizer.js.map