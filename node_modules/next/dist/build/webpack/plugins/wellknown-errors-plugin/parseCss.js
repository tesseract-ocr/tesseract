"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCssError", {
    enumerable: true,
    get: function() {
        return getCssError;
    }
});
const _picocolors = require("../../../../lib/picocolors");
const _simpleWebpackError = require("./simpleWebpackError");
const regexCssError = /^(?:CssSyntaxError|SyntaxError)\n\n\((\d+):(\d*)\) (.*)$/s;
function getCssError(fileName, err) {
    if (!((err.name === 'CssSyntaxError' || err.name === 'SyntaxError') && err.stack === false && !(err instanceof SyntaxError))) {
        return false;
    }
    // https://github.com/postcss/postcss-loader/blob/d6931da177ac79707bd758436e476036a55e4f59/src/Error.js
    const res = regexCssError.exec(err.message);
    if (res) {
        const [, _lineNumber, _column, reason] = res;
        const lineNumber = Math.max(1, parseInt(_lineNumber, 10));
        const column = Math.max(1, parseInt(_column, 10));
        return new _simpleWebpackError.SimpleWebpackError(`${(0, _picocolors.cyan)(fileName)}:${(0, _picocolors.yellow)(lineNumber.toString())}:${(0, _picocolors.yellow)(column.toString())}`, (0, _picocolors.red)((0, _picocolors.bold)('Syntax error')).concat(`: ${reason}`));
    }
    return false;
}

//# sourceMappingURL=parseCss.js.map