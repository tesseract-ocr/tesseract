import { absolutePathToPage } from "../../../shared/lib/page-path/absolute-path-to-page";
/**
 * Normalizes a given filename so that it's relative to the provided directory.
 * It will also strip the extension (if provided) and the trailing `/index`.
 */ export class AbsoluteFilenameNormalizer {
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
        return absolutePathToPage(filename, {
            extensions: this.extensions,
            keepIndex: false,
            dir: this.dir,
            pagesType: this.pagesType
        });
    }
}

//# sourceMappingURL=absolute-filename-normalizer.js.map