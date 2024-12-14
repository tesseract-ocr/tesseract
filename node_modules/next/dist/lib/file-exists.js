"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    FileType: null,
    fileExists: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    FileType: function() {
        return FileType;
    },
    fileExists: function() {
        return fileExists;
    }
});
const _fs = require("fs");
const _iserror = /*#__PURE__*/ _interop_require_default(require("./is-error"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
var FileType = /*#__PURE__*/ function(FileType) {
    FileType["File"] = "file";
    FileType["Directory"] = "directory";
    return FileType;
}({});
async function fileExists(fileName, type) {
    try {
        if (type === "file") {
            const stats = await _fs.promises.stat(fileName);
            return stats.isFile();
        } else if (type === "directory") {
            const stats = await _fs.promises.stat(fileName);
            return stats.isDirectory();
        }
        return (0, _fs.existsSync)(fileName);
    } catch (err) {
        if ((0, _iserror.default)(err) && (err.code === 'ENOENT' || err.code === 'ENAMETOOLONG')) {
            return false;
        }
        throw err;
    }
}

//# sourceMappingURL=file-exists.js.map