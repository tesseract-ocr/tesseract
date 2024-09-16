"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppBundlePathNormalizer: null,
    DevAppBundlePathNormalizer: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppBundlePathNormalizer: function() {
        return AppBundlePathNormalizer;
    },
    DevAppBundlePathNormalizer: function() {
        return DevAppBundlePathNormalizer;
    }
});
const _normalizers = require("../../normalizers");
const _prefixingnormalizer = require("../../prefixing-normalizer");
const _normalizepagepath = require("../../../../../shared/lib/page-path/normalize-page-path");
class AppBundlePathNormalizer extends _prefixingnormalizer.PrefixingNormalizer {
    constructor(){
        super("app");
    }
    normalize(page) {
        return super.normalize((0, _normalizepagepath.normalizePagePath)(page));
    }
}
class DevAppBundlePathNormalizer extends _normalizers.Normalizers {
    constructor(pageNormalizer){
        super([
            // This should normalize the filename to a page.
            pageNormalizer,
            // Normalize the app page to a pathname.
            new AppBundlePathNormalizer()
        ]);
    }
    normalize(filename) {
        return super.normalize(filename);
    }
}

//# sourceMappingURL=app-bundle-path-normalizer.js.map