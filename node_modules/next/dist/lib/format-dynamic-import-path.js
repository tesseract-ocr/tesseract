"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "formatDynamicImportPath", {
    enumerable: true,
    get: function() {
        return formatDynamicImportPath;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _url = require("url");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const formatDynamicImportPath = (dir, filePath)=>{
    const absoluteFilePath = _path.default.isAbsolute(filePath) ? filePath : _path.default.join(dir, filePath);
    const formattedFilePath = (0, _url.pathToFileURL)(absoluteFilePath).toString();
    return formattedFilePath;
};

//# sourceMappingURL=format-dynamic-import-path.js.map