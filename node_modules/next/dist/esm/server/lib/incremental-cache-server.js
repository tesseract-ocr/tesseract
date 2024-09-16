import { createIpcServer } from "./server-ipc";
import { IncrementalCache } from "./incremental-cache";
let initializeResult;
export async function initialize(...constructorArgs) {
    const incrementalCache = new IncrementalCache(...constructorArgs);
    const { ipcPort, ipcValidationKey } = await createIpcServer({
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