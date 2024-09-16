"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getBabelConfigFile", {
    enumerable: true,
    get: function() {
        return getBabelConfigFile;
    }
});
const _path = require("path");
const _fs = require("fs");
const BABEL_CONFIG_FILES = [
    ".babelrc",
    ".babelrc.json",
    ".babelrc.js",
    ".babelrc.mjs",
    ".babelrc.cjs",
    "babel.config.js",
    "babel.config.json",
    "babel.config.mjs",
    "babel.config.cjs"
];
function getBabelConfigFile(dir) {
    for (const filename of BABEL_CONFIG_FILES){
        const configFilePath = (0, _path.join)(dir, filename);
        const exists = (0, _fs.existsSync)(configFilePath);
        if (!exists) {
            continue;
        }
        return configFilePath;
    }
}

//# sourceMappingURL=get-babel-config-file.js.map