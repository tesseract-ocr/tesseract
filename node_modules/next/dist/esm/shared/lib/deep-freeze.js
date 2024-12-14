/**
 * Recursively freezes an object and all of its properties. This prevents the
 * object from being modified at runtime. When the JS runtime is running in
 * strict mode, any attempts to modify a frozen object will throw an error.
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/freeze
 * @param obj The object to freeze.
 */ export function deepFreeze(obj) {
    // If the object is already frozen, there's no need to freeze it again.
    if (Object.isFrozen(obj)) return obj;
    // An array is an object, but we also want to freeze each element in the array
    // as well.
    if (Array.isArray(obj)) {
        for (const item of obj){
            if (!item || typeof item !== 'object') continue;
            deepFreeze(item);
        }
        return Object.freeze(obj);
    }
    for (const value of Object.values(obj)){
        if (!value || typeof value !== 'object') continue;
        deepFreeze(value);
    }
    return Object.freeze(obj);
}

//# sourceMappingURL=deep-freeze.js.map