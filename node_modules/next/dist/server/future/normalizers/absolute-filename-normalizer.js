"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AbsoluteFilenameNormalizer", {
    enumerable: true,
    get: function() {
        return AbsoluteFilenameNormalizer;
    }
});
const _absolutepathtopage = require("../../../shared/lib/page-path/absolute-path-to-page");
class AbsoluteFilenameNormalizer {
    /**
   *
   * @param dir the directory for which the files should be made relative to
   * @param extensions the extensions the file could have
   * @param keepIndex when `true` the trailing `/index` is _not_ removed
   */ constructor(dir, extensions, pagesType){
        this.dir = dir;
        this.extensions = extensions;
        this.pagesType = pagesType;
    }
    normalize(filename) {
        return (0, _absolutepathtopage.absolutePathToPage)(filename, {
            extensions: this.extensions,
            keepIndex: false,
            dir: this.dir,
            pagesType: this.pagesType
        });
    }
}

//# sourceMappingURL=absolute-filename-normalizer.js.map