"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "formatRevalidate", {
    enumerable: true,
    get: function() {
        return formatRevalidate;
    }
});
const _constants = require("../../lib/constants");
function formatRevalidate({ revalidate, swrDelta }) {
    const swrHeader = swrDelta ? `stale-while-revalidate=${swrDelta}` : "stale-while-revalidate";
    if (revalidate === 0) {
        return "private, no-cache, no-store, max-age=0, must-revalidate";
    } else if (typeof revalidate === "number") {
        return `s-maxage=${revalidate}, ${swrHeader}`;
    }
    return `s-maxage=${_constants.CACHE_ONE_YEAR}, ${swrHeader}`;
}

//# sourceMappingURL=revalidate.js.map