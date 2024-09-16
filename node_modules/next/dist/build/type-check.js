"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "startTypeChecking", {
    enumerable: true,
    get: function() {
        return startTypeChecking;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("./output/log"));
const _jestworker = require("next/dist/compiled/jest-worker");
const _verifyAndLint = require("../lib/verifyAndLint");
const _spinner = /*#__PURE__*/ _interop_require_default(require("./spinner"));
const _events = require("../telemetry/events");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
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
 * typescript will be loaded in "next/lib/verify-typescript-setup" and
 * then passed to "next/lib/typescript/runTypeCheck" as a parameter.
 *
 * Since it is impossible to pass a function from main thread to a worker,
 * instead of running "next/lib/typescript/runTypeCheck" in a worker,
 * we will run entire "next/lib/verify-typescript-setup" in a worker instead.
 */ function verifyTypeScriptSetup(dir, distDir, intentDirs, typeCheckPreflight, tsconfigPath, disableStaticImages, cacheDir, enableWorkerThreads, hasAppDir, hasPagesDir) {
    const typeCheckWorker = new _jestworker.Worker(require.resolve("../lib/verify-typescript-setup"), {
        numWorkers: 1,
        enableWorkerThreads,
        maxRetries: 0
    });
    typeCheckWorker.getStdout().pipe(process.stdout);
    typeCheckWorker.getStderr().pipe(process.stderr);
    return typeCheckWorker.verifyTypeScriptSetup({
        dir,
        distDir,
        intentDirs,
        typeCheckPreflight,
        tsconfigPath,
        disableStaticImages,
        cacheDir,
        hasAppDir,
        hasPagesDir
    }).then((result)=>{
        typeCheckWorker.end();
        return result;
    }).catch(()=>{
        // The error is already logged in the worker, we simply exit the main thread to prevent the
        // `Jest worker encountered 1 child process exceptions, exceeding retry limit` from showing up
        process.exit(1);
    });
}
async function startTypeChecking({ cacheDir, config, dir, ignoreESLint, nextBuildSpan, pagesDir, runLint, shouldLint, telemetry, appDir }) {
    const ignoreTypeScriptErrors = Boolean(config.typescript.ignoreBuildErrors);
    const eslintCacheDir = _path.default.join(cacheDir, "eslint/");
    if (ignoreTypeScriptErrors) {
        _log.info("Skipping validation of types");
    }
    if (runLint && ignoreESLint) {
        // only print log when build require lint while ignoreESLint is enabled
        _log.info("Skipping linting");
    }
    let typeCheckingAndLintingSpinnerPrefixText;
    let typeCheckingAndLintingSpinner;
    if (!ignoreTypeScriptErrors && shouldLint) {
        typeCheckingAndLintingSpinnerPrefixText = "Linting and checking validity of types";
    } else if (!ignoreTypeScriptErrors) {
        typeCheckingAndLintingSpinnerPrefixText = "Checking validity of types";
    } else if (shouldLint) {
        typeCheckingAndLintingSpinnerPrefixText = "Linting";
    }
    // we will not create a spinner if both ignoreTypeScriptErrors and ignoreESLint are
    // enabled, but we will still verifying project's tsconfig and dependencies.
    if (typeCheckingAndLintingSpinnerPrefixText) {
        typeCheckingAndLintingSpinner = (0, _spinner.default)(typeCheckingAndLintingSpinnerPrefixText);
    }
    const typeCheckStart = process.hrtime();
    try {
        const [[verifyResult, typeCheckEnd]] = await Promise.all([
            nextBuildSpan.traceChild("verify-typescript-setup").traceAsyncFn(()=>verifyTypeScriptSetup(dir, config.distDir, [
                    pagesDir,
                    appDir
                ].filter(Boolean), !ignoreTypeScriptErrors, config.typescript.tsconfigPath, config.images.disableStaticImages, cacheDir, config.experimental.workerThreads, !!appDir, !!pagesDir).then((resolved)=>{
                    const checkEnd = process.hrtime(typeCheckStart);
                    return [
                        resolved,
                        checkEnd
                    ];
                })),
            shouldLint && nextBuildSpan.traceChild("verify-and-lint").traceAsyncFn(async ()=>{
                var _config_eslint;
                await (0, _verifyAndLint.verifyAndLint)(dir, eslintCacheDir, (_config_eslint = config.eslint) == null ? void 0 : _config_eslint.dirs, config.experimental.workerThreads, telemetry);
            })
        ]);
        typeCheckingAndLintingSpinner == null ? void 0 : typeCheckingAndLintingSpinner.stopAndPersist();
        if (!ignoreTypeScriptErrors && verifyResult) {
            var _verifyResult_result, _verifyResult_result1, _verifyResult_result2;
            telemetry.record((0, _events.eventTypeCheckCompleted)({
                durationInSeconds: typeCheckEnd[0],
                typescriptVersion: verifyResult.version,
                inputFilesCount: (_verifyResult_result = verifyResult.result) == null ? void 0 : _verifyResult_result.inputFilesCount,
                totalFilesCount: (_verifyResult_result1 = verifyResult.result) == null ? void 0 : _verifyResult_result1.totalFilesCount,
                incremental: (_verifyResult_result2 = verifyResult.result) == null ? void 0 : _verifyResult_result2.incremental
            }));
        }
    } catch (err) {
        // prevent showing jest-worker internal error as it
        // isn't helpful for users and clutters output
        if ((0, _iserror.default)(err) && err.message === "Call retries were exceeded") {
            await telemetry.flush();
            process.exit(1);
        }
        throw err;
    }
}

//# sourceMappingURL=type-check.js.map