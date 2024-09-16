// This file should be imported before any others. It sets up the environment
// for later imports to work properly.
// expose AsyncLocalStorage on global for react usage if it isn't already provided by the environment
"use strict";
if (typeof globalThis.AsyncLocalStorage !== "function") {
    const { AsyncLocalStorage } = require("async_hooks");
    globalThis.AsyncLocalStorage = AsyncLocalStorage;
}
if (typeof globalThis.WebSocket !== "function") {
    Object.defineProperty(globalThis, "WebSocket", {
        get () {
            return require("next/dist/compiled/ws").WebSocket;
        }
    });
}

//# sourceMappingURL=node-environment.js.map