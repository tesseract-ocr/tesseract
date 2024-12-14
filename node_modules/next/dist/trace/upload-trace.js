"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return uploadTrace;
    }
});
const _shared = require("./shared");
const _storage = require("../telemetry/storage");
function uploadTrace({ traceUploadUrl, mode, projectDir, distDir, isTurboSession, sync }) {
    const { NEXT_TRACE_UPLOAD_DEBUG } = process.env;
    const telemetry = new _storage.Telemetry({
        distDir
    });
    // Note: cross-spawn is not used here as it causes
    // a new command window to appear when we don't want it to
    const child_process = require('child_process');
    // we use spawnSync when debugging to ensure logs are piped
    // correctly to stdout/stderr
    const spawn = NEXT_TRACE_UPLOAD_DEBUG || sync ? child_process.spawnSync : child_process.spawn;
    spawn(process.execPath, [
        require.resolve('./trace-uploader'),
        traceUploadUrl,
        mode,
        projectDir,
        distDir,
        String(isTurboSession),
        _shared.traceId,
        telemetry.anonymousId,
        telemetry.sessionId
    ], {
        detached: !NEXT_TRACE_UPLOAD_DEBUG,
        windowsHide: true,
        shell: false,
        ...NEXT_TRACE_UPLOAD_DEBUG ? {
            stdio: 'inherit'
        } : {}
    });
}

//# sourceMappingURL=upload-trace.js.map