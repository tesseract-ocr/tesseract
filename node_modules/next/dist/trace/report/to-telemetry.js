"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _shared = require("../shared");
const TRACE_EVENT_ACCESSLIST = new Map(Object.entries({
    'webpack-invalidated': 'WEBPACK_INVALIDATED'
}));
const reportToTelemetry = ({ name, duration })=>{
    const eventName = TRACE_EVENT_ACCESSLIST.get(name);
    if (!eventName) {
        return;
    }
    const telemetry = _shared.traceGlobals.get('telemetry');
    if (!telemetry) {
        return;
    }
    telemetry.record({
        eventName,
        payload: {
            durationInMicroseconds: duration
        }
    });
};
const _default = {
    flushAll: ()=>{},
    report: reportToTelemetry
};

//# sourceMappingURL=to-telemetry.js.map