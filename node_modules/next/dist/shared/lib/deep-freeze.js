"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "deepFreeze", {
    enumerable: true,
    get: function() {
        return deepFreeze;
    }
});
function deepFreeze(obj) {
    // If the object is already frozen, there's no need to freeze it again.
    if (Object.isFrozen(obj)) return obj;
    // An array is an object, but we also want to freeze each element in the array
    // as well.
    if (Array.isArray(obj)) {
        for (const item of obj){
            if (!item || typeof item !== "object") continue;
            deepFreeze(item);
        }
        return Object.freeze(obj);
    }
    for (const value of Object.values(obj)){
        if (!value || typeof value !== "object") continue;
        deepFreeze(value);
    }
    return Object.freeze(obj);
}

//# sourceMappingURL=deep-freeze.js.map