"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    icssParser: null,
    importParser: null,
    urlParser: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    icssParser: function() {
        return _postcssicssparser.default;
    },
    importParser: function() {
        return _postcssimportparser.default;
    },
    urlParser: function() {
        return _postcssurlparser.default;
    }
});
const _postcssimportparser = /*#__PURE__*/ _interop_require_default(require("./postcss-import-parser"));
const _postcssicssparser = /*#__PURE__*/ _interop_require_default(require("./postcss-icss-parser"));
const _postcssurlparser = /*#__PURE__*/ _interop_require_default(require("./postcss-url-parser"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}

//# sourceMappingURL=index.js.map