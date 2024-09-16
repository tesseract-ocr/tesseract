/* eslint-disable import/no-extraneous-dependencies */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "registerServerReference", {
    enumerable: true,
    get: function() {
        return registerServerReference;
    }
});
const _serveredge = require("react-server-dom-webpack/server.edge");
function registerServerReference(id, action) {
    return (0, _serveredge.registerServerReference)(action, id, null);
}

//# sourceMappingURL=server-reference.js.map