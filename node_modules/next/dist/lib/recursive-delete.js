"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "recursiveDelete", {
    enumerable: true,
    get: function() {
        return recursiveDelete;
    }
});
const _fs = require("fs");
const _path = require("path");
const _iserror = /*#__PURE__*/ _interop_require_default(require("./is-error"));
const _wait = require("./wait");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const unlinkPath = async (p, isDir = false, t = 1)=>{
    try {
        if (isDir) {
            await _fs.promises.rmdir(p);
        } else {
            await _fs.promises.unlink(p);
        }
    } catch (e) {
        const code = (0, _iserror.default)(e) && e.code;
        if ((code === "EBUSY" || code === "ENOTEMPTY" || code === "EPERM" || code === "EMFILE") && t < 3) {
            await (0, _wait.wait)(t * 100);
            return unlinkPath(p, isDir, t++);
        }
        if (code === "ENOENT") {
            return;
        }
        throw e;
    }
};
async function recursiveDelete(/** Directory to delete the contents of */ dir, /** Exclude based on relative file path */ exclude, /** Ensures that parameter dir exists, this is not passed recursively */ previousPath = "") {
    let result;
    try {
        result = await _fs.promises.readdir(dir, {
            withFileTypes: true
        });
    } catch (e) {
        if ((0, _iserror.default)(e) && e.code === "ENOENT") {
            return;
        }
        throw e;
    }
    await Promise.all(result.map(async (part)=>{
        const absolutePath = (0, _path.join)(dir, part.name);
        // readdir does not follow symbolic links
        // if part is a symbolic link, follow it using stat
        let isDirectory = part.isDirectory();
        const isSymlink = part.isSymbolicLink();
        if (isSymlink) {
            const linkPath = await _fs.promises.readlink(absolutePath);
            try {
                const stats = await _fs.promises.stat((0, _path.isAbsolute)(linkPath) ? linkPath : (0, _path.join)((0, _path.dirname)(absolutePath), linkPath));
                isDirectory = stats.isDirectory();
            } catch  {}
        }
        const pp = (0, _path.join)(previousPath, part.name);
        const isNotExcluded = !exclude || !exclude.test(pp);
        if (isNotExcluded) {
            if (isDirectory) {
                await recursiveDelete(absolutePath, exclude, pp);
            }
            return unlinkPath(absolutePath, !isSymlink && isDirectory);
        }
    }));
}

//# sourceMappingURL=recursive-delete.js.map