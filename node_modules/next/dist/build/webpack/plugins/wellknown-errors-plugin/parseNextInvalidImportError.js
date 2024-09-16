"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getNextInvalidImportError", {
    enumerable: true,
    get: function() {
        return getNextInvalidImportError;
    }
});
const _getModuleTrace = require("./getModuleTrace");
const _simpleWebpackError = require("./simpleWebpackError");
function getNextInvalidImportError(err, module, compilation, compiler) {
    try {
        if (!module.loaders.find((loader)=>loader.loader.includes("next-invalid-import-error-loader.js"))) {
            return false;
        }
        const { moduleTrace } = (0, _getModuleTrace.getModuleTrace)(module, compilation, compiler);
        const { formattedModuleTrace, lastInternalFileName, invalidImportMessage } = (0, _getModuleTrace.formatModuleTrace)(compiler, moduleTrace);
        return new _simpleWebpackError.SimpleWebpackError(lastInternalFileName, err.message + invalidImportMessage + "\n\nImport trace for requested module:\n" + formattedModuleTrace);
    } catch  {
        return false;
    }
}

//# sourceMappingURL=parseNextInvalidImportError.js.map