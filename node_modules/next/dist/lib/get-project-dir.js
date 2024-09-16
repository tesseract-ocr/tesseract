"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getProjectDir", {
    enumerable: true,
    get: function() {
        return getProjectDir;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _log = require("../build/output/log");
const _detecttypo = require("./detect-typo");
const _realpath = require("./realpath");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getProjectDir(dir) {
    try {
        const resolvedDir = _path.default.resolve(dir || ".");
        const realDir = (0, _realpath.realpathSync)(resolvedDir);
        if (resolvedDir !== realDir && resolvedDir.toLowerCase() === realDir.toLowerCase()) {
            (0, _log.warn)(`Invalid casing detected for project dir, received ${resolvedDir} actual path ${realDir}, see more info here https://nextjs.org/docs/messages/invalid-project-dir-casing`);
        }
        return realDir;
    } catch (err) {
        if (err.code === "ENOENT") {
            if (typeof dir === "string") {
                const detectedTypo = (0, _detecttypo.detectTypo)(dir, [
                    "build",
                    "dev",
                    "info",
                    "lint",
                    "start",
                    "telemetry"
                ]);
                if (detectedTypo) {
                    (0, _log.error)(`"next ${dir}" does not exist. Did you mean "next ${detectedTypo}"?`);
                    process.exit(1);
                }
            }
            (0, _log.error)(`Invalid project directory provided, no such directory: ${_path.default.resolve(dir || ".")}`);
            process.exit(1);
        }
        throw err;
    }
}

//# sourceMappingURL=get-project-dir.js.map