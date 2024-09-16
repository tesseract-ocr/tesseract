"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "verifyPartytownSetup", {
    enumerable: true,
    get: function() {
        return verifyPartytownSetup;
    }
});
const _fs = require("fs");
const _picocolors = require("./picocolors");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _hasnecessarydependencies = require("./has-necessary-dependencies");
const _fileexists = require("./file-exists");
const _fatalerror = require("./fatal-error");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _getpkgmanager = require("./helpers/get-pkg-manager");
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
async function missingDependencyError(dir) {
    const packageManager = (0, _getpkgmanager.getPkgManager)(dir);
    throw new _fatalerror.FatalError((0, _picocolors.bold)((0, _picocolors.red)("It looks like you're trying to use Partytown with next/script but do not have the required package(s) installed.")) + "\n\n" + (0, _picocolors.bold)(`Please install Partytown by running:`) + "\n\n" + `\t${(0, _picocolors.bold)((0, _picocolors.cyan)((packageManager === "yarn" ? "yarn add --dev" : packageManager === "pnpm" ? "pnpm install --save-dev" : "npm install --save-dev") + " @builder.io/partytown"))}` + "\n\n" + (0, _picocolors.bold)(`If you are not trying to use Partytown, please disable the experimental ${(0, _picocolors.cyan)('"nextScriptWorkers"')} flag in next.config.js.`) + "\n");
}
async function copyPartytownStaticFiles(deps, staticDir) {
    const partytownLibDir = _path.default.join(staticDir, "~partytown");
    const hasPartytownLibDir = await (0, _fileexists.fileExists)(partytownLibDir, _fileexists.FileType.Directory);
    if (hasPartytownLibDir) {
        await _fs.promises.rm(partytownLibDir, {
            recursive: true,
            force: true
        });
    }
    const { copyLibFiles } = await Promise.resolve(require(_path.default.join(deps.resolved.get("@builder.io/partytown"), "../utils")));
    await copyLibFiles(partytownLibDir);
}
async function verifyPartytownSetup(dir, targetDir) {
    try {
        var _partytownDeps_missing;
        const partytownDeps = await (0, _hasnecessarydependencies.hasNecessaryDependencies)(dir, [
            {
                file: "@builder.io/partytown",
                pkg: "@builder.io/partytown",
                exportsRestrict: false
            }
        ]);
        if (((_partytownDeps_missing = partytownDeps.missing) == null ? void 0 : _partytownDeps_missing.length) > 0) {
            await missingDependencyError(dir);
        } else {
            try {
                await copyPartytownStaticFiles(partytownDeps, targetDir);
            } catch (err) {
                _log.warn(`Partytown library files could not be copied to the static directory. Please ensure that ${(0, _picocolors.bold)((0, _picocolors.cyan)("@builder.io/partytown"))} is installed as a dependency.`);
            }
        }
    } catch (err) {
        // Don't show a stack trace when there is an error due to missing dependencies
        if (err instanceof _fatalerror.FatalError) {
            console.error(err.message);
            process.exit(1);
        }
        throw err;
    }
}

//# sourceMappingURL=verify-partytown-setup.js.map