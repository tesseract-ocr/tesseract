"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "webpackBuild", {
    enumerable: true,
    get: function() {
        return webpackBuild;
    }
});
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../output/log"));
const _buildcontext = require("../build-context");
const _jestworker = require("next/dist/compiled/jest-worker");
const _debug = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/debug"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _trace = require("../../trace");
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
const debug = (0, _debug.default)("next:build:webpack-build");
const ORDERED_COMPILER_NAMES = [
    "server",
    "edge-server",
    "client"
];
let pluginState = {};
function deepMerge(target, source) {
    const result = {
        ...target,
        ...source
    };
    for (const key of Object.keys(result)){
        result[key] = Array.isArray(target[key]) ? target[key] = [
            ...target[key],
            ...source[key] || []
        ] : typeof target[key] == "object" && typeof source[key] == "object" ? deepMerge(target[key], source[key]) : result[key];
    }
    return result;
}
async function webpackBuildWithWorker(compilerNamesArg) {
    const compilerNames = compilerNamesArg || ORDERED_COMPILER_NAMES;
    const { nextBuildSpan, ...prunedBuildContext } = _buildcontext.NextBuildContext;
    prunedBuildContext.pluginState = pluginState;
    const getWorker = (compilerName)=>{
        var _worker__workerPool;
        const _worker = new _jestworker.Worker(_path.default.join(__dirname, "impl.js"), {
            exposedMethods: [
                "workerMain"
            ],
            numWorkers: 1,
            maxRetries: 0,
            forkOptions: {
                env: {
                    ...process.env,
                    NEXT_PRIVATE_BUILD_WORKER: "1"
                }
            }
        });
        _worker.getStderr().pipe(process.stderr);
        _worker.getStdout().pipe(process.stdout);
        for (const worker of ((_worker__workerPool = _worker._workerPool) == null ? void 0 : _worker__workerPool._workers) || []){
            worker._child.on("exit", (code, signal)=>{
                if (code || signal && signal !== "SIGINT") {
                    debug(`Compiler ${compilerName} unexpectedly exited with code: ${code} and signal: ${signal}`);
                }
            });
        }
        return _worker;
    };
    const combinedResult = {
        duration: 0,
        buildTraceContext: {}
    };
    for (const compilerName of compilerNames){
        var _curResult_buildTraceContext;
        const worker = getWorker(compilerName);
        const curResult = await worker.workerMain({
            buildContext: prunedBuildContext,
            compilerName,
            traceState: {
                ...(0, _trace.exportTraceState)(),
                defaultParentSpanId: nextBuildSpan == null ? void 0 : nextBuildSpan.getId(),
                shouldSaveTraceEvents: true
            }
        });
        if (nextBuildSpan && curResult.debugTraceEvents) {
            (0, _trace.recordTraceEvents)(curResult.debugTraceEvents);
        }
        // destroy worker so it's not sticking around using memory
        await worker.end();
        // Update plugin state
        pluginState = deepMerge(pluginState, curResult.pluginState);
        prunedBuildContext.pluginState = pluginState;
        if (curResult.telemetryState) {
            _buildcontext.NextBuildContext.telemetryState = curResult.telemetryState;
        }
        combinedResult.duration += curResult.duration;
        if ((_curResult_buildTraceContext = curResult.buildTraceContext) == null ? void 0 : _curResult_buildTraceContext.entriesTrace) {
            var _curResult_buildTraceContext1;
            const { entryNameMap } = curResult.buildTraceContext.entriesTrace;
            if (entryNameMap) {
                combinedResult.buildTraceContext.entriesTrace = curResult.buildTraceContext.entriesTrace;
                combinedResult.buildTraceContext.entriesTrace.entryNameMap = entryNameMap;
            }
            if ((_curResult_buildTraceContext1 = curResult.buildTraceContext) == null ? void 0 : _curResult_buildTraceContext1.chunksTrace) {
                const { entryNameFilesMap } = curResult.buildTraceContext.chunksTrace;
                if (entryNameFilesMap) {
                    combinedResult.buildTraceContext.chunksTrace = curResult.buildTraceContext.chunksTrace;
                    combinedResult.buildTraceContext.chunksTrace.entryNameFilesMap = entryNameFilesMap;
                }
            }
        }
    }
    if (compilerNames.length === 3) {
        _log.event("Compiled successfully");
    }
    return combinedResult;
}
function webpackBuild(withWorker, compilerNames) {
    if (withWorker) {
        debug("using separate compiler workers");
        return webpackBuildWithWorker(compilerNames);
    } else {
        debug("building all compilers in same process");
        const webpackBuildImpl = require("./impl").webpackBuildImpl;
        return webpackBuildImpl(null, null);
    }
}

//# sourceMappingURL=index.js.map