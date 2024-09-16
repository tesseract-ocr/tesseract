"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isWriteable", {
    enumerable: true,
    get: function() {
        return isWriteable;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function isWriteable(directory) {
    try {
        await _fs.default.promises.access(directory, (_fs.default.constants || _fs.default).W_OK);
        return true;
    } catch (err) {
        return false;
    }
}

//# sourceMappingURL=is-writeable.js.map