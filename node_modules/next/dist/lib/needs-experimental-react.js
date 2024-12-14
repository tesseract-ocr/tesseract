"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "needsExperimentalReact", {
    enumerable: true,
    get: function() {
        return needsExperimentalReact;
    }
});
function needsExperimentalReact(config) {
    const { ppr, taint, reactOwnerStack } = config.experimental || {};
    return Boolean(ppr || taint || reactOwnerStack);
}

//# sourceMappingURL=needs-experimental-react.js.map