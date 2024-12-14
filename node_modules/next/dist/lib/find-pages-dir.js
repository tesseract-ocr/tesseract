"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    findDir: null,
    findPagesDir: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    findDir: function() {
        return findDir;
    },
    findPagesDir: function() {
        return findPagesDir;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function findDir(dir, name) {
    // prioritize ./${name} over ./src/${name}
    let curDir = _path.default.join(dir, name);
    if (_fs.default.existsSync(curDir)) return curDir;
    curDir = _path.default.join(dir, 'src', name);
    if (_fs.default.existsSync(curDir)) return curDir;
    return null;
}
function findPagesDir(dir) {
    const pagesDir = findDir(dir, 'pages') || undefined;
    const appDir = findDir(dir, 'app') || undefined;
    if (appDir == null && pagesDir == null) {
        throw new Error("> Couldn't find any `pages` or `app` directory. Please create one under the project root");
    }
    return {
        pagesDir,
        appDir
    };
}

//# sourceMappingURL=find-pages-dir.js.map