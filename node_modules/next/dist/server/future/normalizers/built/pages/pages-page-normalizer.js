"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DevPagesPageNormalizer", {
    enumerable: true,
    get: function() {
        return DevPagesPageNormalizer;
    }
});
const _pagetypes = require("../../../../../lib/page-types");
const _absolutefilenamenormalizer = require("../../absolute-filename-normalizer");
class DevPagesPageNormalizer extends _absolutefilenamenormalizer.AbsoluteFilenameNormalizer {
    constructor(pagesDir, extensions){
        super(pagesDir, extensions, _pagetypes.PAGE_TYPES.PAGES);
    }
}

//# sourceMappingURL=pages-page-normalizer.js.map