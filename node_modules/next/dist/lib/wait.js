/**
 * Wait for a given number of milliseconds and then resolve.
 *
 * @param ms the number of milliseconds to wait
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "wait", {
    enumerable: true,
    get: function() {
        return wait;
    }
});
async function wait(ms) {
    return new Promise((resolve)=>setTimeout(resolve, ms));
}

//# sourceMappingURL=wait.js.map