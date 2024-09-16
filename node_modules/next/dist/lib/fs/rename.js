/*
MIT License

Copyright (c) 2015 - present Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */ // This file is based on https://github.com/microsoft/vscode/blob/f860fcf11022f10a992440fd54c6e45674e39617/src/vs/base/node/pfs.ts
// See the LICENSE at the top of the file
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "rename", {
    enumerable: true,
    get: function() {
        return rename;
    }
});
const _gracefulfs = /*#__PURE__*/ _interop_require_wildcard(require("graceful-fs"));
const _util = require("util");
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
async function rename(source, target, windowsRetryTimeout = 60000 /* matches graceful-fs */ ) {
    if (source === target) {
        return; // simulate node.js behaviour here and do a no-op if paths match
    }
    if (process.platform === "win32" && typeof windowsRetryTimeout === "number") {
        // On Windows, a rename can fail when either source or target
        // is locked by AV software. We do leverage graceful-fs to iron
        // out these issues, however in case the target file exists,
        // graceful-fs will immediately return without retry for fs.rename().
        await renameWithRetry(source, target, Date.now(), windowsRetryTimeout);
    } else {
        await (0, _util.promisify)(_gracefulfs.rename)(source, target);
    }
}
async function renameWithRetry(source, target, startTime, retryTimeout, attempt = 0) {
    try {
        return await (0, _util.promisify)(_gracefulfs.rename)(source, target);
    } catch (error) {
        if (error.code !== "EACCES" && error.code !== "EPERM" && error.code !== "EBUSY") {
            throw error // only for errors we think are temporary
            ;
        }
        if (Date.now() - startTime >= retryTimeout) {
            console.error(`[node.js fs] rename failed after ${attempt} retries with error: ${error}`);
            throw error // give up after configurable timeout
            ;
        }
        if (attempt === 0) {
            let abortRetry = false;
            try {
                const stat = await (0, _util.promisify)(_gracefulfs.stat)(target);
                if (!stat.isFile()) {
                    abortRetry = true // if target is not a file, EPERM error may be raised and we should not attempt to retry
                    ;
                }
            } catch (e) {
            // Ignore
            }
            if (abortRetry) {
                throw error;
            }
        }
        // Delay with incremental backoff up to 100ms
        await timeout(Math.min(100, attempt * 10));
        // Attempt again
        return renameWithRetry(source, target, startTime, retryTimeout, attempt + 1);
    }
}
const timeout = (millis)=>new Promise((resolve)=>setTimeout(resolve, millis));

//# sourceMappingURL=rename.js.map