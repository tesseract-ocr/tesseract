"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "withCoalescedInvoke", {
    enumerable: true,
    get: function() {
        return withCoalescedInvoke;
    }
});
const globalInvokeCache = new Map();
function withCoalescedInvoke(func) {
    return async function(key, args) {
        const entry = globalInvokeCache.get(key);
        if (entry) {
            return entry.then((res)=>({
                    isOrigin: false,
                    value: res.value
                }));
        }
        async function __wrapper() {
            return await func.apply(undefined, args);
        }
        const future = __wrapper().then((res)=>{
            globalInvokeCache.delete(key);
            return {
                isOrigin: true,
                value: res
            };
        }).catch((err)=>{
            globalInvokeCache.delete(key);
            return Promise.reject(err);
        });
        globalInvokeCache.set(key, future);
        return future;
    };
}

//# sourceMappingURL=coalesced-function.js.map