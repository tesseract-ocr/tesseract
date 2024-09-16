"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getScssError", {
    enumerable: true,
    get: function() {
        return getScssError;
    }
});
const _picocolors = require("../../../../lib/picocolors");
const _simpleWebpackError = require("./simpleWebpackError");
const regexScssError = /SassError: (.+)\n\s+on line (\d+) [\s\S]*?>> (.+)\n\s*(-+)\^$/m;
function getScssError(fileName, fileContent, err) {
    if (err.name !== "SassError") {
        return false;
    }
    const res = regexScssError.exec(err.message);
    if (res) {
        const [, reason, _lineNumer, backupFrame, columnString] = res;
        const lineNumber = Math.max(1, parseInt(_lineNumer, 10));
        const column = (columnString == null ? void 0 : columnString.length) ?? 1;
        let frame;
        if (fileContent) {
            try {
                const { codeFrameColumns } = require("next/dist/compiled/babel/code-frame");
                frame = codeFrameColumns(fileContent, {
                    start: {
                        line: lineNumber,
                        column
                    }
                }, {
                    forceColor: true
                });
            } catch  {}
        }
        return new _simpleWebpackError.SimpleWebpackError(`${(0, _picocolors.cyan)(fileName)}:${(0, _picocolors.yellow)(lineNumber.toString())}:${(0, _picocolors.yellow)(column.toString())}`, (0, _picocolors.red)((0, _picocolors.bold)("Syntax error")).concat(`: ${reason}\n\n${frame ?? backupFrame}`));
    }
    return false;
}

//# sourceMappingURL=parseScss.js.map