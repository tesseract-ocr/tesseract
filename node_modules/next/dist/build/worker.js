"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    exportPage: null,
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
    exportPage: function() {
        return _worker.default;
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
const _worker = /*#__PURE__*/ _interop_require_default(require("../export/worker"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}

//# sourceMappingURL=worker.js.map