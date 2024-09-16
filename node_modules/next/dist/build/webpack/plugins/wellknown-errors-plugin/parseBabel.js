"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getBabelError", {
    enumerable: true,
    get: function() {
        return getBabelError;
    }
});
const _picocolors = require("../../../../lib/picocolors");
const _simpleWebpackError = require("./simpleWebpackError");
function getBabelError(fileName, err) {
    if (err.code !== "BABEL_PARSE_ERROR") {
        return false;
    }
    // https://github.com/babel/babel/blob/34693d6024da3f026534dd8d569f97ac0109602e/packages/babel-core/src/parser/index.js
    if (err.loc) {
        const lineNumber = Math.max(1, err.loc.line);
        const column = Math.max(1, err.loc.column);
        let message = err.message// Remove file information, which instead is provided by webpack.
        .replace(/^.+?: /, "")// Remove column information from message
        .replace(new RegExp(`[^\\S\\r\\n]*\\(${lineNumber}:${column}\\)[^\\S\\r\\n]*`), "");
        return new _simpleWebpackError.SimpleWebpackError(`${(0, _picocolors.cyan)(fileName)}:${(0, _picocolors.yellow)(lineNumber.toString())}:${(0, _picocolors.yellow)(column.toString())}`, (0, _picocolors.red)((0, _picocolors.bold)("Syntax error")).concat(`: ${message}`));
    }
    return false;
}

//# sourceMappingURL=parseBabel.js.map