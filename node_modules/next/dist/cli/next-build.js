#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextBuild", {
    enumerable: true,
    get: function() {
        return nextBuild;
    }
});
require("../server/lib/cpu-profile");
const _fs = require("fs");
const _picocolors = require("../lib/picocolors");
const _build = /*#__PURE__*/ _interop_require_default(require("../build"));
const _log = require("../build/output/log");
const _utils = require("../server/lib/utils");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
const _getprojectdir = require("../lib/get-project-dir");
const _startup = require("../lib/memory/startup");
const _shutdown = require("../lib/memory/shutdown");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const nextBuild = (options, directory)=>{
    process.on('SIGTERM', ()=>process.exit(143));
    process.on('SIGINT', ()=>process.exit(130));
    const { debug, experimentalDebugMemoryUsage, profile, lint, mangling, experimentalAppOnly, experimentalTurbo, experimentalBuildMode, experimentalUploadTrace } = options;
    let traceUploadUrl;
    if (experimentalUploadTrace && !process.env.NEXT_TRACE_UPLOAD_DISABLED) {
        traceUploadUrl = experimentalUploadTrace;
    }
    if (!lint) {
        (0, _log.warn)('Linting is disabled.');
    }
    if (!mangling) {
        (0, _log.warn)('Mangling is disabled. Note: This may affect performance and should only be used for debugging purposes.');
    }
    if (profile) {
        (0, _log.warn)(`Profiling is enabled. ${(0, _picocolors.italic)('Note: This may affect performance.')}`);
    }
    if (experimentalDebugMemoryUsage) {
        process.env.EXPERIMENTAL_DEBUG_MEMORY_USAGE = '1';
        (0, _startup.enableMemoryDebuggingMode)();
    }
    const dir = (0, _getprojectdir.getProjectDir)(directory);
    if (!(0, _fs.existsSync)(dir)) {
        (0, _utils.printAndExit)(`> No such directory exists as the project root: ${dir}`);
    }
    if (experimentalTurbo) {
        process.env.TURBOPACK = '1';
    }
    return (0, _build.default)(dir, profile, debug || Boolean(process.env.NEXT_DEBUG_BUILD), lint, !mangling, experimentalAppOnly, !!process.env.TURBOPACK, experimentalBuildMode, traceUploadUrl).catch((err)=>{
        if (experimentalDebugMemoryUsage) {
            (0, _shutdown.disableMemoryDebuggingMode)();
        }
        console.error('');
        if ((0, _iserror.default)(err) && (err.code === 'INVALID_RESOLVE_ALIAS' || err.code === 'WEBPACK_ERRORS' || err.code === 'BUILD_OPTIMIZATION_FAILED' || err.code === 'NEXT_EXPORT_ERROR' || err.code === 'NEXT_STATIC_GEN_BAILOUT' || err.code === 'EDGE_RUNTIME_UNSUPPORTED_API')) {
            (0, _utils.printAndExit)(`> ${err.message}`);
        } else {
            console.error('> Build error occurred');
            (0, _utils.printAndExit)(err);
        }
    }).finally(()=>{
        if (experimentalDebugMemoryUsage) {
            (0, _shutdown.disableMemoryDebuggingMode)();
        }
    });
};

//# sourceMappingURL=next-build.js.map