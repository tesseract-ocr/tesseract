"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppNormalizers: null,
    DevAppNormalizers: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppNormalizers: function() {
        return AppNormalizers;
    },
    DevAppNormalizers: function() {
        return DevAppNormalizers;
    }
});
const _appbundlepathnormalizer = require("./app-bundle-path-normalizer");
const _appfilenamenormalizer = require("./app-filename-normalizer");
const _apppagenormalizer = require("./app-page-normalizer");
const _apppathnamenormalizer = require("./app-pathname-normalizer");
class AppNormalizers {
    constructor(distDir){
        this.filename = new _appfilenamenormalizer.AppFilenameNormalizer(distDir);
        this.pathname = new _apppathnamenormalizer.AppPathnameNormalizer();
        this.bundlePath = new _appbundlepathnormalizer.AppBundlePathNormalizer();
    }
}
class DevAppNormalizers {
    constructor(appDir, extensions){
        this.page = new _apppagenormalizer.DevAppPageNormalizer(appDir, extensions);
        this.pathname = new _apppathnamenormalizer.DevAppPathnameNormalizer(this.page);
        this.bundlePath = new _appbundlepathnormalizer.DevAppBundlePathNormalizer(this.page);
    }
}

//# sourceMappingURL=index.js.map