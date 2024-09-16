"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    Span: null,
    SpanStatus: null,
    exportTraceState: null,
    flushAllTraces: null,
    getTraceEvents: null,
    initializeTraceState: null,
    recordTraceEvents: null,
    setGlobal: null,
    trace: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    Span: function() {
        return _trace.Span;
    },
    SpanStatus: function() {
        return _trace.SpanStatus;
    },
    exportTraceState: function() {
        return _trace.exportTraceState;
    },
    flushAllTraces: function() {
        return _trace.flushAllTraces;
    },
    getTraceEvents: function() {
        return _trace.getTraceEvents;
    },
    initializeTraceState: function() {
        return _trace.initializeTraceState;
    },
    recordTraceEvents: function() {
        return _trace.recordTraceEvents;
    },
    setGlobal: function() {
        return _shared.setGlobal;
    },
    trace: function() {
        return _trace.trace;
    }
});
const _trace = require("./trace");
const _shared = require("./shared");

//# sourceMappingURL=index.js.map