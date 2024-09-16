// An internal module to expose the "waitUntil" API to Edge SSR and Edge Route Handler functions.
// This is highly experimental and subject to change.
// We still need a global key to bypass Webpack's layering of modules.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    internal_getCurrentFunctionWaitUntil: null,
    internal_runWithWaitUntil: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    internal_getCurrentFunctionWaitUntil: function() {
        return internal_getCurrentFunctionWaitUntil;
    },
    internal_runWithWaitUntil: function() {
        return internal_runWithWaitUntil;
    }
});
const GLOBAL_KEY = Symbol.for("__next_internal_waitUntil__");
const state = // @ts-ignore
globalThis[GLOBAL_KEY] || // @ts-ignore
(globalThis[GLOBAL_KEY] = {
    waitUntilCounter: 0,
    waitUntilResolve: undefined,
    waitUntilPromise: null
});
// No matter how many concurrent requests are being handled, we want to make sure
// that the final promise is only resolved once all of the waitUntil promises have
// settled.
function resolveOnePromise() {
    state.waitUntilCounter--;
    if (state.waitUntilCounter === 0) {
        state.waitUntilResolve();
        state.waitUntilPromise = null;
    }
}
function internal_getCurrentFunctionWaitUntil() {
    return state.waitUntilPromise;
}
function internal_runWithWaitUntil(fn) {
    const result = fn();
    if (result && typeof result === "object" && "then" in result && "finally" in result && typeof result.then === "function" && typeof result.finally === "function") {
        if (!state.waitUntilCounter) {
            // Create the promise for the next batch of waitUntil calls.
            state.waitUntilPromise = new Promise((resolve)=>{
                state.waitUntilResolve = resolve;
            });
        }
        state.waitUntilCounter++;
        return result.finally(()=>{
            resolveOnePromise();
        });
    }
    return result;
}

//# sourceMappingURL=internal-edge-wait-until.js.map