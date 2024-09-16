"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "logAppDirError", {
    enumerable: true,
    get: function() {
        return logAppDirError;
    }
});
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../lib/is-error"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
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
function logAppDirError(err) {
    if ((0, _iserror.default)(err) && (err == null ? void 0 : err.stack)) {
        const cleanedStack = err.stack.split("\n").map((line)=>// Remove 'webpack-internal:' noise from the path
            line.replace(/(webpack-internal:\/\/\/|file:\/\/)(\(.*\)\/)?/, ""));
        const filteredStack = cleanedStack// Only display stack frames from the user's code
        .filter((line)=>!/next[\\/]dist[\\/]compiled/.test(line) && !/node_modules[\\/]/.test(line) && !/node:internal[\\/]/.test(line));
        if (filteredStack.length === 1) {
            // This is an error that happened outside of user code, keep full stack
            _log.error(`Internal error: ${cleanedStack.join("\n")}`);
        } else {
            _log.error(filteredStack.join("\n"));
        }
        if (typeof err.digest !== "undefined") {
            console.error(`digest: ${JSON.stringify(err.digest)}`);
        }
        if (err.cause) console.error("Cause:", err.cause);
    } else {
        _log.error(err);
    }
}

//# sourceMappingURL=log-app-dir-error.js.map