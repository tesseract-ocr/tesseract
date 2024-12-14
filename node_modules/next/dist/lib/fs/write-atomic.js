"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "writeFileAtomic", {
    enumerable: true,
    get: function() {
        return writeFileAtomic;
    }
});
const _promises = require("fs/promises");
const _rename = require("./rename");
async function writeFileAtomic(filePath, content) {
    const tempPath = filePath + '.tmp.' + Math.random().toString(36).slice(2);
    try {
        await (0, _promises.writeFile)(tempPath, content, 'utf-8');
        await (0, _rename.rename)(tempPath, filePath);
    } catch (e) {
        try {
            await (0, _promises.unlink)(tempPath);
        } catch  {
        // ignore
        }
        throw e;
    }
}

//# sourceMappingURL=write-atomic.js.map