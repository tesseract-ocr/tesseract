"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "clearModuleContext", {
    enumerable: true,
    get: function() {
        return _context.clearModuleContext;
    }
});
0 && __export(require("./sandbox"));
_export_star(require("./sandbox"), exports);
const _context = require("./context");
function _export_star(from, to) {
    Object.keys(from).forEach(function(k) {
        if (k !== "default" && !Object.prototype.hasOwnProperty.call(to, k)) {
            Object.defineProperty(to, k, {
                enumerable: true,
                get: function() {
                    return from[k];
                }
            });
        }
    });
    return from;
}

//# sourceMappingURL=index.js.map