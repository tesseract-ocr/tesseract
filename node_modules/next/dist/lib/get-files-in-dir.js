"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getFilesInDir", {
    enumerable: true,
    get: function() {
        return getFilesInDir;
    }
});
const _path = require("path");
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function getFilesInDir(path) {
    const dir = await _promises.default.opendir(path);
    const results = [];
    for await (const file of dir){
        let resolvedFile = file;
        if (file.isSymbolicLink()) {
            resolvedFile = await _promises.default.stat((0, _path.join)(path, file.name));
        }
        if (resolvedFile.isFile()) {
            results.push(file.name);
        }
    }
    return results;
}

//# sourceMappingURL=get-files-in-dir.js.map