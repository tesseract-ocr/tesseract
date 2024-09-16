"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createIpcServer", {
    enumerable: true,
    get: function() {
        return createIpcServer;
    }
});
const _render = require("../../render");
const _crypto = /*#__PURE__*/ _interop_require_default(require("crypto"));
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../../lib/is-error"));
const _requestutils = require("./request-utils");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function createIpcServer(server) {
    // Generate a random key in memory to validate messages from other processes.
    // This is just a simple guard against other processes attempting to send
    // traffic to the IPC server.
    const ipcValidationKey = _crypto.default.randomBytes(32).toString("hex");
    const ipcServer = require("http").createServer(async (req, res)=>{
        try {
            const url = new URL(req.url || "/", "http://n");
            const key = url.searchParams.get("key");
            if (key !== ipcValidationKey) {
                return res.end();
            }
            const method = url.searchParams.get("method");
            const args = JSON.parse(url.searchParams.get("args") || "[]");
            if (!method || !Array.isArray(args)) {
                return res.end();
            }
            if (typeof server[method] === "function") {
                var _args_;
                if (method === "logErrorWithOriginalStack" && ((_args_ = args[0]) == null ? void 0 : _args_.stack)) {
                    args[0] = (0, _requestutils.deserializeErr)(args[0]);
                }
                let result = await server[method](...args);
                if (result && typeof result === "object" && result.stack) {
                    result = (0, _render.errorToJSON)(result);
                }
                res.end(JSON.stringify(result || ""));
            }
        } catch (err) {
            if ((0, _iserror.default)(err) && err.code !== "ENOENT") {
                console.error(err);
            }
            res.end(JSON.stringify({
                err: {
                    name: err.name,
                    message: err.message,
                    stack: err.stack
                }
            }));
        }
    });
    const ipcPort = await new Promise((resolveIpc)=>{
        ipcServer.listen(0, server.hostname, ()=>{
            const addr = ipcServer.address();
            if (addr && typeof addr === "object") {
                resolveIpc(addr.port);
            }
        });
    });
    return {
        ipcPort,
        ipcServer,
        ipcValidationKey
    };
}

//# sourceMappingURL=index.js.map