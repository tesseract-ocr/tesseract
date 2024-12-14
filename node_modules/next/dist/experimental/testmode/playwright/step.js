"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "step", {
    enumerable: true,
    get: function() {
        return step;
    }
});
const _test = require("@playwright/test");
function isWithRunAsStep(testInfo) {
    return '_runAsStep' in testInfo;
}
async function step(testInfo, props, handler) {
    if (isWithRunAsStep(testInfo)) {
        return testInfo._runAsStep(props, ({ complete })=>handler(complete));
    }
    // Fallback to the `test.step()`.
    let result;
    let reportedError;
    try {
        await _test.test.step(props.title, async ()=>{
            result = await handler(({ error })=>{
                reportedError = error;
                if (reportedError) {
                    throw reportedError;
                }
            });
        });
    } catch (error) {
        if (error !== reportedError) {
            throw error;
        }
    }
    return result;
}

//# sourceMappingURL=step.js.map