#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextTelemetry", {
    enumerable: true,
    get: function() {
        return nextTelemetry;
    }
});
const _picocolors = require("../lib/picocolors");
const _storage = require("../telemetry/storage");
const telemetry = new _storage.Telemetry({
    distDir: process.cwd()
});
let isEnabled = telemetry.isEnabled;
const nextTelemetry = (options, arg)=>{
    if (options.enable || arg === 'enable') {
        telemetry.setEnabled(true);
        isEnabled = true;
        console.log((0, _picocolors.cyan)('Success!'));
    } else if (options.disable || arg === 'disable') {
        const path = telemetry.setEnabled(false);
        if (isEnabled) {
            console.log((0, _picocolors.cyan)(`Your preference has been saved${path ? ` to ${path}` : ''}.`));
        } else {
            console.log((0, _picocolors.yellow)(`Next.js' telemetry collection is already disabled.`));
        }
        isEnabled = false;
    } else {
        console.log((0, _picocolors.bold)('Next.js Telemetry'));
    }
    console.log(`\nStatus: ${isEnabled ? (0, _picocolors.bold)((0, _picocolors.green)('Enabled')) : (0, _picocolors.bold)((0, _picocolors.red)('Disabled'))}`);
    if (isEnabled) {
        console.log('\nNext.js telemetry is completely anonymous. Thank you for participating!');
    } else {
        console.log(`\nYou have opted-out of Next.js' anonymous telemetry program.\nNo data will be collected from your machine.`);
    }
    console.log(`\nLearn more: ${(0, _picocolors.cyan)('https://nextjs.org/telemetry')}`);
};

//# sourceMappingURL=next-telemetry.js.map