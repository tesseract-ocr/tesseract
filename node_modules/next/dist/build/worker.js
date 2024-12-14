"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    exportPages: null,
    getDefinedNamedExports: null,
    hasCustomGetInitialProps: null,
    isPageStatic: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    exportPages: function() {
        return _worker.exportPages;
    },
    getDefinedNamedExports: function() {
        return _utils.getDefinedNamedExports;
    },
    hasCustomGetInitialProps: function() {
        return _utils.hasCustomGetInitialProps;
    },
    isPageStatic: function() {
        return _utils.isPageStatic;
    }
});
require("../server/require-hook");
const _utils = require("./utils");
const _worker = require("../export/worker");

//# sourceMappingURL=worker.js.map