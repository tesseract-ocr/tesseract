import { errorToJSON } from "../../render";
import crypto from "crypto";
import isError from "../../../lib/is-error";
import { deserializeErr } from "./request-utils";
// we can't use process.send as jest-worker relies on
// it already and can cause unexpected message errors
// so we create an IPC server for communicating
export async function createIpcServer(server) {
    // Generate a random key in memory to validate messages from other processes.
    // This is just a simple guard against other processes attempting to send
    // traffic to the IPC server.
    const ipcValidationKey = crypto.randomBytes(32).toString("hex");
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
                    args[0] = deserializeErr(args[0]);
                }
                let result = await server[method](...args);
                if (result && typeof result === "object" && result.stack) {
                    result = errorToJSON(result);
                }
                res.end(JSON.stringify(result || ""));
            }
        } catch (err) {
            if (isError(err) && err.code !== "ENOENT") {
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