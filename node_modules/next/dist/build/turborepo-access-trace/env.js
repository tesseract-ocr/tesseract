"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "envProxy", {
    enumerable: true,
    get: function() {
        return envProxy;
    }
});
function envProxy(envVars) {
    const newEnv = new Proxy(process.env, {
        get: (target, key, receiver)=>{
            envVars.add(key);
            return Reflect.get(target, key, receiver);
        },
        set: (target, key, value)=>{
            return Reflect.set(target, key, value);
        }
    });
    const oldEnv = process.env;
    process.env = newEnv;
    // Return a function that restores the original environment.
    return ()=>{
        process.env = oldEnv;
    };
}

//# sourceMappingURL=env.js.map