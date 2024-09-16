"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "initialize", {
    enumerable: true,
    get: function() {
        return initialize;
    }
});
const _serveripc = require("./server-ipc");
const _incrementalcache = require("./incremental-cache");
let initializeResult;
async function initialize(...constructorArgs) {
    const incrementalCache = new _incrementalcache.IncrementalCache(...constructorArgs);
    const { ipcPort, ipcValidationKey } = await (0, _serveripc.createIpcServer)({
        async revalidateTag (...args) {
            return incrementalCache.revalidateTag(...args);
        },
        async get (...args) {
            return incrementalCache.get(...args);
        },
        async set (...args) {
            return incrementalCache.set(...args);
        },
        async lock (...args) {
            return incrementalCache.lock(...args);
        },
        async unlock (...args) {
            return incrementalCache.unlock(...args);
        }
    });
    return {
        ipcPort,
        ipcValidationKey
    };
}

//# sourceMappingURL=incremental-cache-server.js.map