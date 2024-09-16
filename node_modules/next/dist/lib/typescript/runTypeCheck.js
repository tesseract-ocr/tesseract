"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "runTypeCheck", {
    enumerable: true,
    get: function() {
        return runTypeCheck;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _diagnosticFormatter = require("./diagnosticFormatter");
const _getTypeScriptConfiguration = require("./getTypeScriptConfiguration");
const _writeConfigurationDefaults = require("./writeConfigurationDefaults");
const _compileerror = require("../compile-error");
const _log = require("../../build/output/log");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function runTypeCheck(ts, baseDir, distDir, tsConfigPath, cacheDir, isAppDirEnabled) {
    const effectiveConfiguration = await (0, _getTypeScriptConfiguration.getTypeScriptConfiguration)(ts, tsConfigPath);
    if (effectiveConfiguration.fileNames.length < 1) {
        return {
            hasWarnings: false,
            inputFilesCount: 0,
            totalFilesCount: 0,
            incremental: false
        };
    }
    const requiredConfig = (0, _writeConfigurationDefaults.getRequiredConfiguration)(ts);
    const options = {
        ...requiredConfig,
        ...effectiveConfiguration.options,
        declarationMap: false,
        emitDeclarationOnly: false,
        noEmit: true
    };
    let program;
    let incremental = false;
    if ((options.incremental || options.composite) && cacheDir) {
        if (options.composite) {
            (0, _log.warn)("TypeScript project references are not fully supported. Attempting to build in incremental mode.");
        }
        incremental = true;
        program = ts.createIncrementalProgram({
            rootNames: effectiveConfiguration.fileNames,
            options: {
                ...options,
                composite: false,
                incremental: true,
                tsBuildInfoFile: _path.default.join(cacheDir, ".tsbuildinfo")
            }
        });
    } else {
        program = ts.createProgram(effectiveConfiguration.fileNames, options);
    }
    const result = program.emit();
    // Intended to match:
    // - pages/test.js
    // - pages/apples.test.js
    // - pages/__tests__/a.js
    //
    // But not:
    // - pages/contest.js
    // - pages/other.js
    // - pages/test/a.js
    //
    const regexIgnoredFile = /[\\/]__(?:tests|mocks)__[\\/]|(?<=[\\/.])(?:spec|test)\.[^\\/]+$/;
    const allDiagnostics = ts.getPreEmitDiagnostics(program).concat(result.diagnostics).filter((d)=>!(d.file && regexIgnoredFile.test(d.file.fileName)));
    const firstError = allDiagnostics.find((d)=>d.category === _diagnosticFormatter.DiagnosticCategory.Error && Boolean(d.file)) ?? allDiagnostics.find((d)=>d.category === _diagnosticFormatter.DiagnosticCategory.Error);
    // In test mode, we want to check all diagnostics, not just the first one.
    if (process.env.__NEXT_TEST_MODE) {
        if (firstError) {
            const allErrors = allDiagnostics.filter((d)=>d.category === _diagnosticFormatter.DiagnosticCategory.Error).map((d)=>"[Test Mode] " + (0, _diagnosticFormatter.getFormattedDiagnostic)(ts, baseDir, distDir, d, isAppDirEnabled));
            console.error("\n\n===== TS errors =====\n\n" + allErrors.join("\n\n") + "\n\n===== TS errors =====\n\n");
            // Make sure all stdout is flushed before we exit.
            await new Promise((resolve)=>setTimeout(resolve, 100));
        }
    }
    if (firstError) {
        throw new _compileerror.CompileError((0, _diagnosticFormatter.getFormattedDiagnostic)(ts, baseDir, distDir, firstError, isAppDirEnabled));
    }
    const warnings = allDiagnostics.filter((d)=>d.category === _diagnosticFormatter.DiagnosticCategory.Warning).map((d)=>(0, _diagnosticFormatter.getFormattedDiagnostic)(ts, baseDir, distDir, d, isAppDirEnabled));
    return {
        hasWarnings: true,
        warnings,
        inputFilesCount: effectiveConfiguration.fileNames.length,
        totalFilesCount: program.getSourceFiles().length,
        incremental
    };
}

//# sourceMappingURL=runTypeCheck.js.map