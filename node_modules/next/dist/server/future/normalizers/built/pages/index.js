"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    DevPagesNormalizers: null,
    PagesNormalizers: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    DevPagesNormalizers: function() {
        return DevPagesNormalizers;
    },
    PagesNormalizers: function() {
        return PagesNormalizers;
    }
});
const _pagesbundlepathnormalizer = require("./pages-bundle-path-normalizer");
const _pagesfilenamenormalizer = require("./pages-filename-normalizer");
const _pagespagenormalizer = require("./pages-page-normalizer");
const _pagespathnamenormalizer = require("./pages-pathname-normalizer");
class PagesNormalizers {
    constructor(distDir){
        this.filename = new _pagesfilenamenormalizer.PagesFilenameNormalizer(distDir);
        this.bundlePath = new _pagesbundlepathnormalizer.PagesBundlePathNormalizer();
    // You'd think that we'd require a `pathname` normalizer here, but for
    // `/pages` we have to handle i18n routes, which means that we need to
    // analyze the page path to determine the locale prefix and it's locale.
    }
}
class DevPagesNormalizers {
    constructor(pagesDir, extensions){
        this.page = new _pagespagenormalizer.DevPagesPageNormalizer(pagesDir, extensions);
        this.pathname = new _pagespathnamenormalizer.DevPagesPathnameNormalizer(pagesDir, extensions);
        this.bundlePath = new _pagesbundlepathnormalizer.DevPagesBundlePathNormalizer(this.page);
    }
}

//# sourceMappingURL=index.js.map