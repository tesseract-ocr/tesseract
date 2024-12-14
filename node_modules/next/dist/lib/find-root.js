"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    findRootDir: null,
    findRootLockFile: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    findRootDir: function() {
        return findRootDir;
    },
    findRootLockFile: function() {
        return findRootLockFile;
    }
});
const _path = require("path");
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function findRootLockFile(cwd) {
    return _findup.default.sync([
        'pnpm-lock.yaml',
        'package-lock.json',
        'yarn.lock',
        'bun.lockb'
    ], {
        cwd
    });
}
function findRootDir(cwd) {
    const lockFile = findRootLockFile(cwd);
    return lockFile ? (0, _path.dirname)(lockFile) : undefined;
}

//# sourceMappingURL=find-root.js.map