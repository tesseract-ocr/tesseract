"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "eventSwcLoadFailure", {
    enumerable: true,
    get: function() {
        return eventSwcLoadFailure;
    }
});
const _shared = require("../../trace/shared");
const _packagejson = require("next/package.json");
const EVENT_PLUGIN_PRESENT = 'NEXT_SWC_LOAD_FAILURE';
async function eventSwcLoadFailure(event) {
    const telemetry = _shared.traceGlobals.get('telemetry');
    // can't continue if telemetry isn't set
    if (!telemetry) return;
    let glibcVersion;
    let installedSwcPackages;
    try {
        var _process_report;
        // @ts-ignore
        glibcVersion = (_process_report = process.report) == null ? void 0 : _process_report.getReport().header.glibcVersionRuntime;
    } catch  {}
    try {
        const pkgNames = Object.keys(_packagejson.optionalDependencies || {}).filter((pkg)=>pkg.startsWith('@next/swc'));
        const installedPkgs = [];
        for (const pkg of pkgNames){
            try {
                const { version } = require(`${pkg}/package.json`);
                installedPkgs.push(`${pkg}@${version}`);
            } catch  {}
        }
        if (installedPkgs.length > 0) {
            installedSwcPackages = installedPkgs.sort().join(',');
        }
    } catch  {}
    telemetry.record({
        eventName: EVENT_PLUGIN_PRESENT,
        payload: {
            nextVersion: _packagejson.version,
            glibcVersion,
            installedSwcPackages,
            arch: process.arch,
            platform: process.platform,
            nodeVersion: process.versions.node,
            wasm: event == null ? void 0 : event.wasm,
            nativeBindingsErrorCode: event == null ? void 0 : event.nativeBindingsErrorCode
        }
    });
    // ensure this event is flushed before process exits
    await telemetry.flush();
}

//# sourceMappingURL=swc-load-failure.js.map