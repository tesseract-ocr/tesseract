"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    hasNextSupport: null,
    isCI: null,
    name: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    hasNextSupport: function() {
        return hasNextSupport;
    },
    isCI: function() {
        return isCI;
    },
    name: function() {
        return name;
    }
});
const _ciinfo = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/ci-info"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const { isCI: _isCI, name: _name } = _ciinfo.default;
const isZeitNow = !!process.env.NOW_BUILDER;
const envStack = process.env.STACK;
const isHeroku = typeof envStack === "string" && envStack.toLowerCase().includes("heroku");
const isCI = isZeitNow || isHeroku || _isCI;
const name = isZeitNow ? "ZEIT Now" : isHeroku ? "Heroku" : _name;
const hasNextSupport = Boolean(isZeitNow);

//# sourceMappingURL=ci-info.js.map