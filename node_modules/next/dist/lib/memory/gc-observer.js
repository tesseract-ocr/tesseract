"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getGcEvents: null,
    startObservingGc: null,
    stopObservingGc: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getGcEvents: function() {
        return getGcEvents;
    },
    startObservingGc: function() {
        return startObservingGc;
    },
    stopObservingGc: function() {
        return stopObservingGc;
    }
});
const _perf_hooks = require("perf_hooks");
const _log = require("../../build/output/log");
const _picocolors = require("../picocolors");
const LONG_RUNNING_GC_THRESHOLD_MS = 15;
const gcEvents = [];
const obs = new _perf_hooks.PerformanceObserver((list)=>{
    const entry = list.getEntries()[0];
    gcEvents.push(entry);
    if (entry.duration > LONG_RUNNING_GC_THRESHOLD_MS) {
        (0, _log.warn)((0, _picocolors.bold)(`Long running GC detected: ${entry.duration.toFixed(2)}ms`));
    }
});
function startObservingGc() {
    obs.observe({
        entryTypes: [
            'gc'
        ]
    });
}
function stopObservingGc() {
    obs.disconnect();
}
function getGcEvents() {
    return gcEvents;
}

//# sourceMappingURL=gc-observer.js.map