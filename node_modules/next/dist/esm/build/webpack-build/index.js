import * as Log from "../output/log";
import { NextBuildContext } from "../build-context";
import { Worker } from "next/dist/compiled/jest-worker";
import origDebug from "next/dist/compiled/debug";
import path from "path";
import { exportTraceState, recordTraceEvents } from "../../trace";
const debug = origDebug("next:build:webpack-build");
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
    const { nextBuildSpan, ...prunedBuildContext } = NextBuildContext;
    prunedBuildContext.pluginState = pluginState;
    const getWorker = (compilerName)=>{
        var _worker__workerPool;
        const _worker = new Worker(path.join(__dirname, "impl.js"), {
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
                ...exportTraceState(),
                defaultParentSpanId: nextBuildSpan == null ? void 0 : nextBuildSpan.getId(),
                shouldSaveTraceEvents: true
            }
        });
        if (nextBuildSpan && curResult.debugTraceEvents) {
            recordTraceEvents(curResult.debugTraceEvents);
        }
        // destroy worker so it's not sticking around using memory
        await worker.end();
        // Update plugin state
        pluginState = deepMerge(pluginState, curResult.pluginState);
        prunedBuildContext.pluginState = pluginState;
        if (curResult.telemetryState) {
            NextBuildContext.telemetryState = curResult.telemetryState;
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
        Log.event("Compiled successfully");
    }
    return combinedResult;
}
export function webpackBuild(withWorker, compilerNames) {
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