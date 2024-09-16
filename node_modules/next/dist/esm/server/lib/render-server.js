import next from "../next";
let initializations = {};
let sandboxContext;
let requireCacheHotReloader;
if (process.env.NODE_ENV !== "production") {
    sandboxContext = require("../web/sandbox/context");
    requireCacheHotReloader = require("../../build/webpack/plugins/nextjs-require-cache-hot-reloader");
}
export function clearAllModuleContexts() {
    return sandboxContext == null ? void 0 : sandboxContext.clearAllModuleContexts();
}
export function clearModuleContext(target) {
    return sandboxContext == null ? void 0 : sandboxContext.clearModuleContext(target);
}
export function deleteAppClientCache() {
    return requireCacheHotReloader == null ? void 0 : requireCacheHotReloader.deleteAppClientCache();
}
export function deleteCache(filePaths) {
    for (const filePath of filePaths){
        requireCacheHotReloader == null ? void 0 : requireCacheHotReloader.deleteCache(filePath);
    }
}
export async function propagateServerField(dir, field, value) {
    const initialization = await initializations[dir];
    if (!initialization) {
        throw new Error("Invariant cant propagate server field, no app initialized");
    }
    const { app } = initialization;
    let appField = app.server;
    if (appField) {
        if (typeof appField[field] === "function") {
            await appField[field].apply(app.server, Array.isArray(value) ? value : []);
        } else {
            appField[field] = value;
        }
    }
}
async function initializeImpl(opts) {
    const type = process.env.__NEXT_PRIVATE_RENDER_WORKER;
    if (type) {
        process.title = "next-render-worker-" + type;
    }
    let requestHandler;
    let upgradeHandler;
    const app = next({
        ...opts,
        hostname: opts.hostname || "localhost",
        customServer: false,
        httpServer: opts.server,
        port: opts.port,
        isNodeDebugging: opts.isNodeDebugging
    });
    requestHandler = app.getRequestHandler();
    upgradeHandler = app.getUpgradeHandler();
    await app.prepare(opts.serverFields);
    return {
        requestHandler,
        upgradeHandler,
        app
    };
}
export async function initialize(opts) {
    // if we already setup the server return as we only need to do
    // this on first worker boot
    if (initializations[opts.dir]) {
        return initializations[opts.dir];
    }
    return initializations[opts.dir] = initializeImpl(opts);
}

//# sourceMappingURL=render-server.js.map