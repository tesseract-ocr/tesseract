"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    downloadNativeNextSwc: null,
    downloadWasmSwc: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    downloadNativeNextSwc: function() {
        return downloadNativeNextSwc;
    },
    downloadWasmSwc: function() {
        return downloadWasmSwc;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _tar = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/tar"));
const _getregistry = require("./helpers/get-registry");
const _getcachedirectory = require("./helpers/get-cache-directory");
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
const { WritableStream } = require("node:stream/web");
const MAX_VERSIONS_TO_CACHE = 8;
async function extractBinary(outputDirectory, pkgName, tarFileName) {
    const cacheDirectory = (0, _getcachedirectory.getCacheDirectory)("next-swc", process.env["NEXT_SWC_PATH"]);
    const extractFromTar = ()=>_tar.default.x({
            file: _path.default.join(cacheDirectory, tarFileName),
            cwd: outputDirectory,
            strip: 1
        });
    if (!_fs.default.existsSync(_path.default.join(cacheDirectory, tarFileName))) {
        _log.info(`Downloading swc package ${pkgName}...`);
        await _fs.default.promises.mkdir(cacheDirectory, {
            recursive: true
        });
        const tempFile = _path.default.join(cacheDirectory, `${tarFileName}.temp-${Date.now()}`);
        const registry = (0, _getregistry.getRegistry)();
        const downloadUrl = `${registry}${pkgName}/-/${tarFileName}`;
        await fetch(downloadUrl).then((res)=>{
            const { ok, body } = res;
            if (!ok || !body) {
                _log.error(`Failed to download swc package from ${downloadUrl}`);
            }
            if (!ok) {
                throw new Error(`request failed with status ${res.status}`);
            }
            if (!body) {
                throw new Error("request failed with empty body");
            }
            const cacheWriteStream = _fs.default.createWriteStream(tempFile);
            return body.pipeTo(new WritableStream({
                write (chunk) {
                    return new Promise((resolve, reject)=>cacheWriteStream.write(chunk, (error)=>{
                            if (error) {
                                reject(error);
                                return;
                            }
                            resolve();
                        }));
                },
                close () {
                    return new Promise((resolve, reject)=>cacheWriteStream.close((error)=>{
                            if (error) {
                                reject(error);
                                return;
                            }
                            resolve();
                        }));
                }
            }));
        });
        await _fs.default.promises.rename(tempFile, _path.default.join(cacheDirectory, tarFileName));
    }
    await extractFromTar();
    const cacheFiles = await _fs.default.promises.readdir(cacheDirectory);
    if (cacheFiles.length > MAX_VERSIONS_TO_CACHE) {
        cacheFiles.sort((a, b)=>{
            if (a.length < b.length) return -1;
            return a.localeCompare(b);
        });
        // prune oldest versions in cache
        for(let i = 0; i++; i < cacheFiles.length - MAX_VERSIONS_TO_CACHE){
            await _fs.default.promises.unlink(_path.default.join(cacheDirectory, cacheFiles[i])).catch(()=>{});
        }
    }
}
async function downloadNativeNextSwc(version, bindingsDirectory, triplesABI) {
    for (const triple of triplesABI){
        const pkgName = `@next/swc-${triple}`;
        const tarFileName = `${pkgName.substring(6)}-${version}.tgz`;
        const outputDirectory = _path.default.join(bindingsDirectory, pkgName);
        if (_fs.default.existsSync(outputDirectory)) {
            // if the package is already downloaded a different
            // failure occurred than not being present
            return;
        }
        await _fs.default.promises.mkdir(outputDirectory, {
            recursive: true
        });
        await extractBinary(outputDirectory, pkgName, tarFileName);
    }
}
async function downloadWasmSwc(version, wasmDirectory, variant = "nodejs") {
    const pkgName = `@next/swc-wasm-${variant}`;
    const tarFileName = `${pkgName.substring(6)}-${version}.tgz`;
    const outputDirectory = _path.default.join(wasmDirectory, pkgName);
    if (_fs.default.existsSync(outputDirectory)) {
        // if the package is already downloaded a different
        // failure occurred than not being present
        return;
    }
    await _fs.default.promises.mkdir(outputDirectory, {
        recursive: true
    });
    await extractBinary(outputDirectory, pkgName, tarFileName);
}

//# sourceMappingURL=download-swc.js.map