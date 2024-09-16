"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    DevPagesBundlePathNormalizer: null,
    PagesBundlePathNormalizer: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    DevPagesBundlePathNormalizer: function() {
        return DevPagesBundlePathNormalizer;
    },
    PagesBundlePathNormalizer: function() {
        return PagesBundlePathNormalizer;
    }
});
const _normalizepagepath = require("../../../../../shared/lib/page-path/normalize-page-path");
const _normalizers = require("../../normalizers");
const _prefixingnormalizer = require("../../prefixing-normalizer");
const _wrapnormalizerfn = require("../../wrap-normalizer-fn");
class PagesBundlePathNormalizer extends _normalizers.Normalizers {
    constructor(){
        super([
            // The bundle path should have the trailing `/index` stripped from
            // it.
            (0, _wrapnormalizerfn.wrapNormalizerFn)(_normalizepagepath.normalizePagePath),
            // The page should prefixed with `pages/`.
            new _prefixingnormalizer.PrefixingNormalizer("pages")
        ]);
    }
    normalize(page) {
        return super.normalize(page);
    }
}
class DevPagesBundlePathNormalizer extends _normalizers.Normalizers {
    constructor(pagesNormalizer){
        super([
            // This should normalize the filename to a page.
            pagesNormalizer,
            // Normalize the app page to a pathname.
            new PagesBundlePathNormalizer()
        ]);
    }
    normalize(filename) {
        return super.normalize(filename);
    }
}

//# sourceMappingURL=pages-bundle-path-normalizer.js.map