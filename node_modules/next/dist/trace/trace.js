"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    Span: null,
    SpanStatus: null,
    clearTraceEvents: null,
    exportTraceState: null,
    flushAllTraces: null,
    getTraceEvents: null,
    initializeTraceState: null,
    recordTraceEvents: null,
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
        return Span;
    },
    SpanStatus: function() {
        return SpanStatus;
    },
    clearTraceEvents: function() {
        return clearTraceEvents;
    },
    exportTraceState: function() {
        return exportTraceState;
    },
    flushAllTraces: function() {
        return flushAllTraces;
    },
    getTraceEvents: function() {
        return getTraceEvents;
    },
    initializeTraceState: function() {
        return initializeTraceState;
    },
    recordTraceEvents: function() {
        return recordTraceEvents;
    },
    trace: function() {
        return trace;
    }
});
const _report = require("./report");
const NUM_OF_MICROSEC_IN_NANOSEC = BigInt("1000");
const NUM_OF_MILLISEC_IN_NANOSEC = BigInt("1000000");
let count = 0;
const getId = ()=>{
    count++;
    return count;
};
let defaultParentSpanId;
let shouldSaveTraceEvents;
let savedTraceEvents = [];
var SpanStatus;
(function(SpanStatus) {
    SpanStatus["Started"] = "started";
    SpanStatus["Stopped"] = "stopped";
})(SpanStatus || (SpanStatus = {}));
class Span {
    constructor({ name, parentId, attrs, startTime }){
        this.name = name;
        this.parentId = parentId ?? defaultParentSpanId;
        this.attrs = attrs ? {
            ...attrs
        } : {};
        if (this.parentId === undefined) {
            // Attach additional information to root spans
            this.attrs.isTurbopack = Boolean(process.env.TURBOPACK);
        }
        this.status = "started";
        this.id = getId();
        this._start = startTime || process.hrtime.bigint();
        // hrtime cannot be used to reconstruct tracing span's actual start time
        // since it does not have relation to clock time:
        // `These times are relative to an arbitrary time in the past, and not related to the time of day and therefore not subject to clock drift`
        // https://nodejs.org/api/process.html#processhrtimetime
        // Capturing current datetime as additional metadata for external reconstruction.
        this.now = Date.now();
    }
    // Durations are reported as microseconds. This gives 1000x the precision
    // of something like Date.now(), which reports in milliseconds.
    // Additionally, ~285 years can be safely represented as microseconds as
    // a float64 in both JSON and JavaScript.
    stop(stopTime) {
        if (this.status === "stopped") {
            // Don't report the same span twice.
            // TODO: In the future this should throw as `.stop()` shouldn't be called multiple times.
            return;
        }
        const end = stopTime || process.hrtime.bigint();
        const duration = (end - this._start) / NUM_OF_MICROSEC_IN_NANOSEC;
        this.status = "stopped";
        if (duration > Number.MAX_SAFE_INTEGER) {
            throw new Error(`Duration is too long to express as float64: ${duration}`);
        }
        const timestamp = this._start / NUM_OF_MICROSEC_IN_NANOSEC;
        const traceEvent = {
            name: this.name,
            duration: Number(duration),
            timestamp: Number(timestamp),
            id: this.id,
            parentId: this.parentId,
            tags: this.attrs,
            startTime: this.now
        };
        _report.reporter.report(traceEvent);
        if (shouldSaveTraceEvents) {
            savedTraceEvents.push(traceEvent);
        }
    }
    traceChild(name, attrs) {
        return new Span({
            name,
            parentId: this.id,
            attrs
        });
    }
    manualTraceChild(name, // Start time in nanoseconds since epoch.
    startTime, // Stop time in nanoseconds since epoch.
    stopTime, attrs) {
        // We need to convert the time info to the same base as hrtime since that is used usually.
        const correction = process.hrtime.bigint() - BigInt(Date.now()) * NUM_OF_MILLISEC_IN_NANOSEC;
        const span = new Span({
            name,
            parentId: this.id,
            attrs,
            startTime: startTime ? startTime + correction : process.hrtime.bigint()
        });
        span.stop(stopTime ? stopTime + correction : process.hrtime.bigint());
    }
    getId() {
        return this.id;
    }
    setAttribute(key, value) {
        this.attrs[key] = value;
    }
    traceFn(fn) {
        try {
            return fn(this);
        } finally{
            this.stop();
        }
    }
    async traceAsyncFn(fn) {
        try {
            return await fn(this);
        } finally{
            this.stop();
        }
    }
}
const trace = (name, parentId, attrs)=>{
    return new Span({
        name,
        parentId,
        attrs
    });
};
const flushAllTraces = ()=>_report.reporter.flushAll();
const exportTraceState = ()=>({
        defaultParentSpanId,
        lastId: count,
        shouldSaveTraceEvents
    });
const initializeTraceState = (state)=>{
    count = state.lastId;
    defaultParentSpanId = state.defaultParentSpanId;
    shouldSaveTraceEvents = state.shouldSaveTraceEvents;
};
function getTraceEvents() {
    return savedTraceEvents;
}
function recordTraceEvents(events) {
    for (const traceEvent of events){
        _report.reporter.report(traceEvent);
        if (traceEvent.id > count) {
            count = traceEvent.id + 1;
        }
    }
    if (shouldSaveTraceEvents) {
        savedTraceEvents.push(...events);
    }
}
const clearTraceEvents = ()=>savedTraceEvents = [];

//# sourceMappingURL=trace.js.map