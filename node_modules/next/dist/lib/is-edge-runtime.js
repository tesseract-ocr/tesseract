"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isEdgeRuntime", {
    enumerable: true,
    get: function() {
        return isEdgeRuntime;
    }
});
const _constants = require("./constants");
function isEdgeRuntime(value) {
    return value === _constants.SERVER_RUNTIME.experimentalEdge || value === _constants.SERVER_RUNTIME.edge;
}

//# sourceMappingURL=is-edge-runtime.js.map