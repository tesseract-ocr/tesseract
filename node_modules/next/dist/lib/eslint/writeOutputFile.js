"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "writeOutputFile", {
    enumerable: true,
    get: function() {
        return writeOutputFile;
    }
});
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../lib/is-error"));
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
/**
 * Check if a given file path is a directory or not.
 * Returns `true` if the path is a directory.
 */ function isDirectory(/**  The path to a file to check. */ filePath) {
    return _fs.promises.stat(filePath).then((stat)=>stat.isDirectory()).catch((error)=>{
        if ((0, _iserror.default)(error) && (error.code === "ENOENT" || error.code === "ENOTDIR")) {
            return false;
        }
        throw error;
    });
}
async function writeOutputFile(/** The name file that needs to be created */ outputFile, /** The data that needs to be inserted into the file */ outputData) {
    const filePath = _path.default.resolve(process.cwd(), outputFile);
    if (await isDirectory(filePath)) {
        _log.error(`Cannot write to output file path, it is a directory: ${filePath}`);
    } else {
        try {
            await _fs.promises.mkdir(_path.default.dirname(filePath), {
                recursive: true
            });
            await _fs.promises.writeFile(filePath, outputData);
            _log.info(`The output file has been created: ${filePath}`);
        } catch (err) {
            _log.error(`There was a problem writing the output file: ${filePath}`);
            console.error(err);
        }
    }
}

//# sourceMappingURL=writeOutputFile.js.map