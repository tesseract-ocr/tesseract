"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return createSpinner;
    }
});
const _ora = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/ora"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("./output/log"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const dotsSpinner = {
    frames: [
        '.',
        '..',
        '...'
    ],
    interval: 200
};
function createSpinner(text, options = {}, logFn = console.log) {
    let spinner;
    let prefixText = ` ${_log.prefixes.info} ${text} `;
    if (process.stdout.isTTY) {
        spinner = (0, _ora.default)({
            text: undefined,
            prefixText,
            spinner: dotsSpinner,
            stream: process.stdout,
            ...options
        }).start();
        // Add capturing of console.log/warn/error to allow pausing
        // the spinner before logging and then restarting spinner after
        const origLog = console.log;
        const origWarn = console.warn;
        const origError = console.error;
        const origStop = spinner.stop.bind(spinner);
        const origStopAndPersist = spinner.stopAndPersist.bind(spinner);
        const logHandle = (method, args)=>{
            // Enter a new line before logging new message, to avoid
            // the new message shows up right after the spinner in the same line.
            const isInProgress = spinner == null ? void 0 : spinner.isSpinning;
            if (spinner && isInProgress) {
                // Reset the current running spinner to empty line by `\r`
                spinner.prefixText = '\r';
                spinner.text = '\r';
                spinner.clear();
                origStop();
            }
            method(...args);
            if (spinner && isInProgress) {
                spinner.start();
            }
        };
        console.log = (...args)=>logHandle(origLog, args);
        console.warn = (...args)=>logHandle(origWarn, args);
        console.error = (...args)=>logHandle(origError, args);
        const resetLog = ()=>{
            console.log = origLog;
            console.warn = origWarn;
            console.error = origError;
        };
        spinner.setText = (newText)=>{
            text = newText;
            prefixText = ` ${_log.prefixes.info} ${newText} `;
            spinner.prefixText = prefixText;
            return spinner;
        };
        spinner.stop = ()=>{
            origStop();
            resetLog();
            return spinner;
        };
        spinner.stopAndPersist = ()=>{
            // Add \r at beginning to reset the current line of loading status text
            const suffixText = `\r ${_log.prefixes.event} ${text} `;
            if (spinner) {
                spinner.text = suffixText;
            } else {
                logFn(suffixText);
            }
            origStopAndPersist();
            resetLog();
            return spinner;
        };
    } else if (prefixText || text) {
        logFn(prefixText ? prefixText + '...' : text);
    }
    return spinner;
}

//# sourceMappingURL=spinner.js.map