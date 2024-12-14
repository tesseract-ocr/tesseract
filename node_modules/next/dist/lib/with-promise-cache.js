"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "withPromiseCache", {
    enumerable: true,
    get: function() {
        return withPromiseCache;
    }
});
function withPromiseCache(cache, fn, getKey) {
    return (...values)=>{
        const key = getKey ? getKey(...values) : values[0];
        let p = cache.get(key);
        if (!p) {
            p = fn(...values);
            p.catch(()=>cache.remove(key));
            cache.set(key, p);
        }
        return p;
    };
}

//# sourceMappingURL=with-promise-cache.js.map