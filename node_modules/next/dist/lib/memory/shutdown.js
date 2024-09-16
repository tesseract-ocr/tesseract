"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "disableMemoryDebuggingMode", {
    enumerable: true,
    get: function() {
        return disableMemoryDebuggingMode;
    }
});
const _log = require("../../build/output/log");
const _picocolors = require("../picocolors");
const _gcobserver = require("./gc-observer");
const _trace = require("./trace");
function disableMemoryDebuggingMode() {
    (0, _trace.stopPeriodicMemoryUsageTracing)();
    (0, _gcobserver.stopObservingGc)();
    (0, _log.info)((0, _picocolors.bold)("Memory usage report:"));
    const gcEvents = (0, _gcobserver.getGcEvents)();
    const totalTimeInGcMs = gcEvents.reduce((acc, event)=>acc + event.duration, 0);
    (0, _log.info)(` - Total time spent in GC: ${totalTimeInGcMs.toFixed(2)}ms`);
    const allMemoryUsage = (0, _trace.getAllMemoryUsageSpans)();
    const peakHeapUsage = Math.max(...allMemoryUsage.map((usage)=>usage["memory.heapUsed"]));
    const peakRssUsage = Math.max(...allMemoryUsage.map((usage)=>usage["memory.rss"]));
    (0, _log.info)(` - Peak heap usage: ${(peakHeapUsage / 1024 / 1024).toFixed(2)} MB`);
    (0, _log.info)(` - Peak RSS usage: ${(peakRssUsage / 1024 / 1024).toFixed(2)} MB`);
}

//# sourceMappingURL=shutdown.js.map