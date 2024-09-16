"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getDynamicCodeEvaluationError", {
    enumerable: true,
    get: function() {
        return getDynamicCodeEvaluationError;
    }
});
const _getModuleTrace = require("./getModuleTrace");
const _simpleWebpackError = require("./simpleWebpackError");
function getDynamicCodeEvaluationError(message, module, compilation, compiler) {
    const { moduleTrace } = (0, _getModuleTrace.getModuleTrace)(module, compilation, compiler);
    const { formattedModuleTrace, lastInternalFileName, invalidImportMessage } = (0, _getModuleTrace.formatModuleTrace)(compiler, moduleTrace);
    return new _simpleWebpackError.SimpleWebpackError(lastInternalFileName, message + invalidImportMessage + "\n\nImport trace for requested module:\n" + formattedModuleTrace);
}

//# sourceMappingURL=parse-dynamic-code-evaluation-error.js.map