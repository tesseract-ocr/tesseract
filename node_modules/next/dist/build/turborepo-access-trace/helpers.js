"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    turborepoTraceAccess: null,
    writeTurborepoAccessTraceResult: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    turborepoTraceAccess: function() {
        return turborepoTraceAccess;
    },
    writeTurborepoAccessTraceResult: function() {
        return writeTurborepoAccessTraceResult;
    }
});
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _env = require("./env");
const _tcp = require("./tcp");
const _result = require("./result");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function turborepoTraceAccess(f, parent) {
    // If the trace file is not set, don't trace and instead just call the
    // function.
    if (!process.env.TURBOREPO_TRACE_FILE) return f();
    // Otherwise, trace the function and merge the results into the parent. Using
    // `then` instead of `await` here to avoid creating a new async context when
    // tracing is disabled.
    return withTurborepoTraceAccess(f).then(([result, proxy])=>{
        parent.merge(proxy);
        // Return the result of the function.
        return result;
    });
}
async function writeTurborepoAccessTraceResult({ distDir, traces }) {
    const configTraceFile = process.env.TURBOREPO_TRACE_FILE;
    if (!configTraceFile || traces.length === 0) return;
    // merge traces
    const [accessTrace, ...otherTraces] = traces;
    for (const trace of otherTraces){
        accessTrace.merge(trace);
    }
    try {
        // make sure the directory exists
        await _promises.default.mkdir(_path.default.dirname(configTraceFile), {
            recursive: true
        });
        await _promises.default.writeFile(configTraceFile, JSON.stringify({
            outputs: [
                `${distDir}/**`,
                `!${distDir}/cache/**`
            ],
            accessed: accessTrace.toPublicTrace()
        }));
    } catch (err) {
        // if we can't write this file, we should bail out here to avoid
        // the possibility of incorrect turborepo cache hits.
        throw new Error(`Failed to write turborepo access trace file`, {
            cause: err
        });
    }
}
async function withTurborepoTraceAccess(f) {
    const envVars = new Set([]);
    // addresses is an array of objects, so a set is useless
    const addresses = [];
    // TODO: watch fsPaths (removed from this implementation for now)
    const fsPaths = new Set();
    // setup proxies
    const restoreTCP = (0, _tcp.tcpProxy)(addresses);
    const restoreEnv = (0, _env.envProxy)(envVars);
    let functionResult;
    // NOTE: we intentionally don't catch errors here so the calling function can handle them
    try {
        // call the wrapped function
        functionResult = await f();
    } finally{
        // remove proxies
        restoreTCP();
        restoreEnv();
    }
    const traceResult = new _result.TurborepoAccessTraceResult(envVars, addresses, fsPaths);
    return [
        functionResult,
        traceResult
    ];
}

//# sourceMappingURL=helpers.js.map