"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DevPagesPathnameNormalizer", {
    enumerable: true,
    get: function() {
        return DevPagesPathnameNormalizer;
    }
});
const _pagetypes = require("../../../../../lib/page-types");
const _absolutefilenamenormalizer = require("../../absolute-filename-normalizer");
class DevPagesPathnameNormalizer extends _absolutefilenamenormalizer.AbsoluteFilenameNormalizer {
    constructor(pagesDir, extensions){
        super(pagesDir, extensions, _pagetypes.PAGE_TYPES.PAGES);
    }
}

//# sourceMappingURL=pages-pathname-normalizer.js.map