"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _child_process = /*#__PURE__*/ _interop_require_default(require("child_process"));
const _assert = /*#__PURE__*/ _interop_require_default(require("assert"));
const _nodefetch = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/node-fetch"));
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _readline = require("readline");
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _storage = require("../telemetry/storage");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
// Predefined set of the event names to be included in the trace.
// If the trace span's name matches to one of the event names in the set,
// it'll up uploaded to the trace server.
const EVENT_FILTER = new Set([
    "client-hmr-latency",
    "hot-reloader",
    "webpack-invalid-client",
    "webpack-invalidated-server",
    "navigation-to-hydration",
    "start-dev-server",
    "compile-path",
    "memory-usage",
    "server-restart-close-to-memory-threshold"
]);
const { NEXT_TRACE_UPLOAD_DEBUG, // An external env to allow to upload full trace without picking up the relavant spans.
// This is mainly for the debugging purpose, to allwo manual audit for full trace for the given build.
// [NOTE] This may fail if build is large and generated trace is excessively large.
NEXT_TRACE_UPLOAD_FULL } = process.env;
const isDebugEnabled = !!NEXT_TRACE_UPLOAD_DEBUG || !!NEXT_TRACE_UPLOAD_FULL;
const shouldUploadFullTrace = !!NEXT_TRACE_UPLOAD_FULL;
const [, , traceUploadUrl, mode, projectDir, distDir] = process.argv;
(async function upload() {
    const nextVersion = JSON.parse(await _promises.default.readFile(_path.default.resolve(__dirname, "../../package.json"), "utf8")).version;
    const telemetry = new _storage.Telemetry({
        distDir
    });
    const projectPkgJsonPath = await (0, _findup.default)("package.json");
    (0, _assert.default)(projectPkgJsonPath);
    const projectPkgJson = JSON.parse(await _promises.default.readFile(projectPkgJsonPath, "utf-8"));
    const pkgName = projectPkgJson.name;
    const commit = _child_process.default.spawnSync(_os.default.platform() === "win32" ? "git.exe" : "git", [
        "rev-parse",
        "HEAD"
    ], {
        shell: true
    }).stdout.toString().trimEnd();
    const readLineInterface = (0, _readline.createInterface)({
        input: (0, _fs.createReadStream)(_path.default.join(projectDir, distDir, "trace")),
        crlfDelay: Infinity
    });
    let isTurboSession = false;
    const traces = new Map();
    for await (const line of readLineInterface){
        const lineEvents = JSON.parse(line);
        for (const event of lineEvents){
            if (// Always include root spans
            event.parentId === undefined || shouldUploadFullTrace || EVENT_FILTER.has(event.name)) {
                let trace = traces.get(event.traceId);
                if (trace === undefined) {
                    trace = [];
                    traces.set(event.traceId, trace);
                }
                if (typeof event.tags.isTurbopack === "boolean") {
                    isTurboSession = event.tags.isTurbopack;
                }
                trace.push(event);
            }
        }
    }
    const body = {
        metadata: {
            anonymousId: telemetry.anonymousId,
            arch: _os.default.arch(),
            commit,
            cpus: _os.default.cpus().length,
            isTurboSession,
            mode,
            nextVersion,
            pkgName,
            platform: _os.default.platform(),
            sessionId: telemetry.sessionId
        },
        traces: [
            ...traces.values()
        ]
    };
    if (isDebugEnabled) {
        console.log("Sending request with body", JSON.stringify(body, null, 2));
    }
    let res = await (0, _nodefetch.default)(traceUploadUrl, {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "x-trace-transfer-mode": shouldUploadFullTrace ? "full" : "default"
        },
        body: JSON.stringify(body)
    });
    if (isDebugEnabled) {
        console.log("Received response", res.status, await res.json());
    }
})();

//# sourceMappingURL=trace-uploader.js.map