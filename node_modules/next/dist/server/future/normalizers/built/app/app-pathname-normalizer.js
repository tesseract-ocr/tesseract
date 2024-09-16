"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppPathnameNormalizer: null,
    DevAppPathnameNormalizer: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppPathnameNormalizer: function() {
        return AppPathnameNormalizer;
    },
    DevAppPathnameNormalizer: function() {
        return DevAppPathnameNormalizer;
    }
});
const _apppaths = require("../../../../../shared/lib/router/utils/app-paths");
const _normalizers = require("../../normalizers");
const _wrapnormalizerfn = require("../../wrap-normalizer-fn");
const _underscorenormalizer = require("../../underscore-normalizer");
class AppPathnameNormalizer extends _normalizers.Normalizers {
    constructor(){
        super([
            // The pathname to match should have the trailing `/page` and other route
            // group information stripped from it.
            (0, _wrapnormalizerfn.wrapNormalizerFn)(_apppaths.normalizeAppPath),
            // The page should have the `%5F` characters replaced with `_` characters.
            new _underscorenormalizer.UnderscoreNormalizer()
        ]);
    }
    normalize(page) {
        return super.normalize(page);
    }
}
class DevAppPathnameNormalizer extends _normalizers.Normalizers {
    constructor(pageNormalizer){
        super([
            // This should normalize the filename to a page.
            pageNormalizer,
            // Normalize the app page to a pathname.
            new AppPathnameNormalizer()
        ]);
    }
    normalize(filename) {
        return super.normalize(filename);
    }
}

//# sourceMappingURL=app-pathname-normalizer.js.map