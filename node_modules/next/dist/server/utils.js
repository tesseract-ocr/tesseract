"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    cleanAmpPath: null,
    debounce: null,
    isBlockedPage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    cleanAmpPath: function() {
        return cleanAmpPath;
    },
    debounce: function() {
        return debounce;
    },
    isBlockedPage: function() {
        return isBlockedPage;
    }
});
const _constants = require("../shared/lib/constants");
function isBlockedPage(page) {
    return _constants.BLOCKED_PAGES.includes(page);
}
function cleanAmpPath(pathname) {
    if (pathname.match(/\?amp=(y|yes|true|1)/)) {
        pathname = pathname.replace(/\?amp=(y|yes|true|1)&?/, "?");
    }
    if (pathname.match(/&amp=(y|yes|true|1)/)) {
        pathname = pathname.replace(/&amp=(y|yes|true|1)/, "");
    }
    pathname = pathname.replace(/\?$/, "");
    return pathname;
}
function debounce(fn, ms, maxWait = Infinity) {
    let timeoutId;
    // The time the debouncing function was first called during this debounce queue.
    let startTime = 0;
    // The time the debouncing function was last called.
    let lastCall = 0;
    // The arguments and this context of the last call to the debouncing function.
    let args, context;
    // A helper used to that either invokes the debounced function, or
    // reschedules the timer if a more recent call was made.
    function run() {
        const now = Date.now();
        const diff = lastCall + ms - now;
        // If the diff is non-positive, then we've waited at least `ms`
        // milliseconds since the last call. Or if we've waited for longer than the
        // max wait time, we must call the debounced function.
        if (diff <= 0 || startTime + maxWait >= now) {
            // It's important to clear the timeout id before invoking the debounced
            // function, in case the function calls the debouncing function again.
            timeoutId = undefined;
            fn.apply(context, args);
        } else {
            // Else, a new call was made after the original timer was scheduled. We
            // didn't clear the timeout (doing so is very slow), so now we need to
            // reschedule the timer for the time difference.
            timeoutId = setTimeout(run, diff);
        }
    }
    return function(...passedArgs) {
        // The arguments and this context of the most recent call are saved so the
        // debounced function can be invoked with them later.
        args = passedArgs;
        context = this;
        // Instead of constantly clearing and scheduling a timer, we record the
        // time of the last call. If a second call comes in before the timer fires,
        // then we'll reschedule in the run function. Doing this is considerably
        // faster.
        lastCall = Date.now();
        // Only schedule a new timer if we're not currently waiting.
        if (timeoutId === undefined) {
            startTime = lastCall;
            timeoutId = setTimeout(run, ms);
        }
    };
}

//# sourceMappingURL=utils.js.map