"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
const _promises = require("fs/promises");
const _ = require(".");
const _shared = require("../shared");
const _path = require("path");
const _os = require("os");
const TRACE_EVENT = {
    name: 'test-span',
    duration: 321,
    timestamp: Date.now(),
    id: 127,
    startTime: Date.now()
};
const WEBPACK_INVALIDATED_EVENT = {
    name: 'webpack-invalidated',
    duration: 100,
    timestamp: Date.now(),
    id: 112,
    startTime: Date.now()
};
describe('Trace Reporter', ()=>{
    describe('JSON reporter', ()=>{
        it('should write the trace events to JSON file', async ()=>{
            const tmpDir = await (0, _promises.mkdtemp)((0, _path.join)((0, _os.tmpdir)(), 'json-reporter'));
            (0, _shared.setGlobal)('distDir', tmpDir);
            (0, _shared.setGlobal)('phase', 'anything');
            _.reporter.report(TRACE_EVENT);
            await _.reporter.flushAll();
            const traceFilename = (0, _path.join)(tmpDir, 'trace');
            const traces = JSON.parse(await (0, _promises.readFile)(traceFilename, 'utf-8'));
            expect(traces.length).toEqual(1);
            expect(traces[0].name).toEqual('test-span');
            expect(traces[0].id).toEqual(127);
            expect(traces[0].duration).toEqual(321);
            expect(traces[0].traceId).toBeDefined();
        });
    });
    describe('Telemetry reporter', ()=>{
        it('should record telemetry event', async ()=>{
            const recordMock = jest.fn();
            const telemetryMock = {
                record: recordMock
            };
            (0, _shared.setGlobal)('telemetry', telemetryMock);
            // This should be ignored.
            _.reporter.report(TRACE_EVENT);
            expect(recordMock).toHaveBeenCalledTimes(0);
            _.reporter.report(WEBPACK_INVALIDATED_EVENT);
            expect(recordMock).toHaveBeenCalledTimes(1);
            expect(recordMock).toHaveBeenCalledWith({
                eventName: 'WEBPACK_INVALIDATED',
                payload: {
                    durationInMicroseconds: 100
                }
            });
        });
    });
});

//# sourceMappingURL=index.test.js.map