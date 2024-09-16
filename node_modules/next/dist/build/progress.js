"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createProgress", {
    enumerable: true,
    get: function() {
        return createProgress;
    }
});
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _spinner = /*#__PURE__*/ _interop_require_default(require("./spinner"));
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
function divideSegments(number, segments) {
    const result = [];
    while(number > 0 && segments > 0){
        const dividedNumber = number < segments ? number : Math.floor(number / segments);
        number -= dividedNumber;
        segments--;
        result.push(dividedNumber);
    }
    return result;
}
const createProgress = (total, label)=>{
    const segments = divideSegments(total, 4);
    if (total === 0) {
        throw new Error("invariant: progress total can not be zero");
    }
    let currentSegmentTotal = segments.shift();
    let currentSegmentCount = 0;
    let lastProgressOutput = Date.now();
    let curProgress = 0;
    let progressSpinner = (0, _spinner.default)(`${label} (${curProgress}/${total})`, {
        spinner: {
            frames: [
                "[    ]",
                "[=   ]",
                "[==  ]",
                "[=== ]",
                "[ ===]",
                "[  ==]",
                "[   =]",
                "[    ]",
                "[   =]",
                "[  ==]",
                "[ ===]",
                "[====]",
                "[=== ]",
                "[==  ]",
                "[=   ]"
            ],
            interval: 200
        }
    });
    return ()=>{
        curProgress++;
        // Make sure we only log once
        // - per fully generated segment, or
        // - per minute
        // when not showing the spinner
        if (!progressSpinner) {
            currentSegmentCount++;
            if (currentSegmentCount === currentSegmentTotal) {
                currentSegmentTotal = segments.shift();
                currentSegmentCount = 0;
            } else if (lastProgressOutput + 60000 > Date.now()) {
                return;
            }
            lastProgressOutput = Date.now();
        }
        const isFinished = curProgress === total;
        const message = `${label} (${curProgress}/${total})`;
        if (progressSpinner && !isFinished) {
            progressSpinner.setText(message);
        } else {
            progressSpinner == null ? void 0 : progressSpinner.stop();
            if (isFinished) {
                _log.event(message);
            } else {
                _log.info(`${message} ${process.stdout.isTTY ? "\n" : "\r"}`);
            }
        }
    };
};

//# sourceMappingURL=progress.js.map