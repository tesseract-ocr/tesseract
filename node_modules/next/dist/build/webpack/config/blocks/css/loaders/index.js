"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && __export(require("./global")) && __export(require("./modules"));
_export_star(require("./global"), exports);
_export_star(require("./modules"), exports);
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