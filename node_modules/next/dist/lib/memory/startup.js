"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "enableMemoryDebuggingMode", {
    enumerable: true,
    get: function() {
        return enableMemoryDebuggingMode;
    }
});
const _v8 = /*#__PURE__*/ _interop_require_default(require("v8"));
const _log = require("../../build/output/log");
const _picocolors = require("../picocolors");
const _gcobserver = require("./gc-observer");
const _trace = require("./trace");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function enableMemoryDebuggingMode() {
    // This will generate a heap snapshot when the program is close to the
    // memory limit. It does not give any warning to the user though which
    // can be jarring. If memory is large, this may take a long time.
    if ("setHeapSnapshotNearHeapLimit" in _v8.default) {
        // @ts-expect-error - this method exists since Node 16.
        _v8.default.setHeapSnapshotNearHeapLimit(1);
    }
    // This flag will kill the process when it starts to GC thrash when it's
    // close to the memory limit rather than continuing to try to collect
    // memory ineffectively.
    _v8.default.setFlagsFromString("--detect-ineffective-gcs-near-heap-limit");
    // This allows users to generate a heap snapshot on demand just by sending
    // a signal to the process.
    process.on("SIGUSR2", ()=>{
        (0, _log.warn)(`Received SIGUSR2 signal. Generating heap snapshot. ${(0, _picocolors.italic)("Note: this will take some time.")}`);
        _v8.default.writeHeapSnapshot();
    });
    (0, _gcobserver.startObservingGc)();
    (0, _trace.startPeriodicMemoryUsageTracing)();
    (0, _log.warn)(`Memory debugging mode is enabled. ${(0, _picocolors.italic)("Note: This will affect performance.")}`);
    (0, _log.info)(" - Heap snapshots will be automatically generated when the process reaches more than 70% of the memory limit and again when the process is just about to run out of memory.");
    (0, _log.info)(` - To manually generate a heap snapshot, send the process a SIGUSR2 signal: \`kill -SIGUSR2 ${process.pid}\``);
    (0, _log.info)(" - Heap snapshots when there is high memory will take a very long time to complete and may be difficult to analyze in tools.");
    (0, _log.info)(" - See https://nextjs.org/docs/app/building-your-application/optimizing/memory-usage for more information.");
}

//# sourceMappingURL=startup.js.map