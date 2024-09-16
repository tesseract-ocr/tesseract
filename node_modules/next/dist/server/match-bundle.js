"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return matchBundle;
    }
});
const _getroutefromassetpath = /*#__PURE__*/ _interop_require_default(require("../shared/lib/router/utils/get-route-from-asset-path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function matchBundle(regex, input) {
    const result = regex.exec(input);
    if (!result) {
        return null;
    }
    return (0, _getroutefromassetpath.default)(`/${result[1]}`);
}

//# sourceMappingURL=match-bundle.js.map