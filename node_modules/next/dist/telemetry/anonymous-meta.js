"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getAnonymousMeta", {
    enumerable: true,
    get: function() {
        return getAnonymousMeta;
    }
});
const _isdocker = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/is-docker"));
const _iswsl = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/is-wsl"));
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _ciinfo = /*#__PURE__*/ _interop_require_wildcard(require("../server/ci-info"));
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
let traits;
function getAnonymousMeta() {
    if (traits) {
        return traits;
    }
    const cpus = _os.default.cpus() || [];
    const { NOW_REGION } = process.env;
    traits = {
        // Software information
        systemPlatform: _os.default.platform(),
        systemRelease: _os.default.release(),
        systemArchitecture: _os.default.arch(),
        // Machine information
        cpuCount: cpus.length,
        cpuModel: cpus.length ? cpus[0].model : null,
        cpuSpeed: cpus.length ? cpus[0].speed : null,
        memoryInMb: Math.trunc(_os.default.totalmem() / Math.pow(1024, 2)),
        // Environment information
        isDocker: (0, _isdocker.default)(),
        isNowDev: NOW_REGION === 'dev1',
        isWsl: _iswsl.default,
        isCI: _ciinfo.isCI,
        ciName: _ciinfo.isCI && _ciinfo.name || null,
        nextVersion: "15.1.0"
    };
    return traits;
}

//# sourceMappingURL=anonymous-meta.js.map