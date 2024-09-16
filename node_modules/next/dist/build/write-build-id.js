"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "writeBuildId", {
    enumerable: true,
    get: function() {
        return writeBuildId;
    }
});
const _fs = require("fs");
const _path = require("path");
const _constants = require("../shared/lib/constants");
async function writeBuildId(distDir, buildId) {
    const buildIdPath = (0, _path.join)(distDir, _constants.BUILD_ID_FILE);
    await _fs.promises.writeFile(buildIdPath, buildId, "utf8");
}

//# sourceMappingURL=write-build-id.js.map