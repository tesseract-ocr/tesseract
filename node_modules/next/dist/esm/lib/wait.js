/**
 * Wait for a given number of milliseconds and then resolve.
 *
 * @param ms the number of milliseconds to wait
 */ export async function wait(ms) {
    return new Promise((resolve)=>setTimeout(resolve, ms));
}

//# sourceMappingURL=wait.js.map