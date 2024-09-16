"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "writeDefaultConfig", {
    enumerable: true,
    get: function() {
        return writeDefaultConfig;
    }
});
const _fs = require("fs");
const _picocolors = require("../picocolors");
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _commentjson = /*#__PURE__*/ _interop_require_wildcard(require("next/dist/compiled/comment-json"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
async function writeDefaultConfig(baseDir, { exists, emptyEslintrc, emptyPkgJsonConfig }, selectedConfig, eslintrcFile, pkgJsonPath, packageJsonConfig) {
    if (!exists && emptyEslintrc && eslintrcFile) {
        const ext = _path.default.extname(eslintrcFile);
        let newFileContent;
        if (ext === ".yaml" || ext === ".yml") {
            newFileContent = "extends: 'next'";
        } else {
            newFileContent = _commentjson.stringify(selectedConfig, null, 2);
            if (ext === ".js") {
                newFileContent = "module.exports = " + newFileContent;
            }
        }
        await _fs.promises.writeFile(eslintrcFile, newFileContent + _os.default.EOL);
        _log.info(`We detected an empty ESLint configuration file (${(0, _picocolors.bold)(_path.default.basename(eslintrcFile))}) and updated it for you!`);
    } else if (!exists && emptyPkgJsonConfig && packageJsonConfig) {
        packageJsonConfig.eslintConfig = selectedConfig;
        if (pkgJsonPath) await _fs.promises.writeFile(pkgJsonPath, _commentjson.stringify(packageJsonConfig, null, 2) + _os.default.EOL);
        _log.info(`We detected an empty ${(0, _picocolors.bold)("eslintConfig")} field in package.json and updated it for you!`);
    } else if (!exists) {
        await _fs.promises.writeFile(_path.default.join(baseDir, ".eslintrc.json"), _commentjson.stringify(selectedConfig, null, 2) + _os.default.EOL);
        console.log((0, _picocolors.green)(`We created the ${(0, _picocolors.bold)(".eslintrc.json")} file for you and included your selected configuration.`));
    }
}

//# sourceMappingURL=writeDefaultConfig.js.map