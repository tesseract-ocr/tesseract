/* eslint-disable no-redeclare */ export function withPromiseCache(cache, fn, getKey) {
    return (...values)=>{
        const key = getKey ? getKey(...values) : values[0];
        let p = cache.get(key);
        if (!p) {
            p = fn(...values);
            p.catch(()=>cache.del(key));
            cache.set(key, p);
        }
        return p;
    };
}

//# sourceMappingURL=with-promise-cache.js.map