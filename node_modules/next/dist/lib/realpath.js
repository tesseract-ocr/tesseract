"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "realpathSync", {
    enumerable: true,
    get: function() {
        return realpathSync;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const isWindows = process.platform === 'win32';
const realpathSync = isWindows ? _fs.default.realpathSync : _fs.default.realpathSync.native;

//# sourceMappingURL=realpath.js.map