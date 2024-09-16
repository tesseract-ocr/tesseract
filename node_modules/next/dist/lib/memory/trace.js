"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getAllMemoryUsageSpans: null,
    startPeriodicMemoryUsageTracing: null,
    stopPeriodicMemoryUsageTracing: null,
    traceMemoryUsage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getAllMemoryUsageSpans: function() {
        return getAllMemoryUsageSpans;
    },
    startPeriodicMemoryUsageTracing: function() {
        return startPeriodicMemoryUsageTracing;
    },
    stopPeriodicMemoryUsageTracing: function() {
        return stopPeriodicMemoryUsageTracing;
    },
    traceMemoryUsage: function() {
        return traceMemoryUsage;
    }
});
const _v8 = /*#__PURE__*/ _interop_require_default(require("v8"));
const _log = require("../../build/output/log");
const _trace = require("../../trace");
const _picocolors = require("../picocolors");
const _path = require("path");
const _shared = require("../../trace/shared");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const HEAP_SNAPSHOT_THRESHOLD_PERCENT = 70;
let alreadyGeneratedHeapSnapshot = false;
const TRACE_MEMORY_USAGE_TIMER_MS = 20000;
let traceMemoryUsageTimer;
const allMemoryUsage = [];
function startPeriodicMemoryUsageTracing() {
    traceMemoryUsageTimer = setTimeout(()=>{
        traceMemoryUsage("periodic memory snapshot");
        startPeriodicMemoryUsageTracing();
    }, TRACE_MEMORY_USAGE_TIMER_MS);
}
function stopPeriodicMemoryUsageTracing() {
    if (traceMemoryUsageTimer) {
        clearTimeout(traceMemoryUsageTimer);
    }
}
function getAllMemoryUsageSpans() {
    return allMemoryUsage;
}
function traceMemoryUsage(description, parentSpan) {
    const memoryUsage = process.memoryUsage();
    const v8HeapStatistics = _v8.default.getHeapStatistics();
    const heapUsed = v8HeapStatistics.used_heap_size;
    const heapMax = v8HeapStatistics.heap_size_limit;
    const tracedMemoryUsage = {
        "memory.rss": memoryUsage.rss,
        "memory.heapUsed": heapUsed,
        "memory.heapTotal": memoryUsage.heapTotal,
        "memory.heapMax": heapMax
    };
    allMemoryUsage.push(tracedMemoryUsage);
    const tracedMemoryUsageAsStrings = Object.fromEntries(Object.entries(tracedMemoryUsage).map(([key, value])=>[
            key,
            String(value)
        ]));
    if (parentSpan) {
        parentSpan.traceChild("memory-usage", tracedMemoryUsageAsStrings);
    } else {
        (0, _trace.trace)("memory-usage", undefined, tracedMemoryUsageAsStrings);
    }
    if (process.env.EXPERIMENTAL_DEBUG_MEMORY_USAGE) {
        const percentageHeapUsed = 100 * heapUsed / heapMax;
        (0, _log.info)("");
        (0, _log.info)("***************************************");
        (0, _log.info)(`Memory usage report at "${description}":`);
        (0, _log.info)(` - RSS: ${(memoryUsage.rss / 1024 / 1024).toFixed(2)} MB`);
        (0, _log.info)(` - Heap Used: ${(heapUsed / 1024 / 1024).toFixed(2)} MB`);
        (0, _log.info)(` - Heap Total Allocated: ${(memoryUsage.heapTotal / 1024 / 1024).toFixed(2)} MB`);
        (0, _log.info)(` - Heap Max: ${(heapMax / 1024 / 1024).toFixed(2)} MB`);
        (0, _log.info)(` - Percentage Heap Used: ${percentageHeapUsed.toFixed(2)}%`);
        (0, _log.info)("***************************************");
        (0, _log.info)("");
        if (percentageHeapUsed > HEAP_SNAPSHOT_THRESHOLD_PERCENT) {
            const distDir = _shared.traceGlobals.get("distDir");
            const heapFilename = (0, _path.join)(distDir, `${description.replace(" ", "-")}.heapsnapshot`);
            (0, _log.warn)((0, _picocolors.bold)(`Heap usage is close to the limit. ${percentageHeapUsed.toFixed(2)}% of heap has been used.`));
            if (!alreadyGeneratedHeapSnapshot) {
                (0, _log.warn)((0, _picocolors.bold)(`Saving heap snapshot to ${heapFilename}.  ${(0, _picocolors.italic)("Note: this will take some time.")}`));
                _v8.default.writeHeapSnapshot(heapFilename);
                alreadyGeneratedHeapSnapshot = true;
            } else {
                (0, _log.warn)("Skipping heap snapshot generation since heap snapshot has already been generated.");
            }
        }
    }
}

//# sourceMappingURL=trace.js.map