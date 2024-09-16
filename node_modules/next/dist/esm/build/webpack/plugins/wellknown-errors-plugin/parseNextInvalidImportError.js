import { formatModuleTrace, getModuleTrace } from "./getModuleTrace";
import { SimpleWebpackError } from "./simpleWebpackError";
export function getNextInvalidImportError(err, module, compilation, compiler) {
    try {
        if (!module.loaders.find((loader)=>loader.loader.includes("next-invalid-import-error-loader.js"))) {
            return false;
        }
        const { moduleTrace } = getModuleTrace(module, compilation, compiler);
        const { formattedModuleTrace, lastInternalFileName, invalidImportMessage } = formatModuleTrace(compiler, moduleTrace);
        return new SimpleWebpackError(lastInternalFileName, err.message + invalidImportMessage + "\n\nImport trace for requested module:\n" + formattedModuleTrace);
    } catch  {
        return false;
    }
}

//# sourceMappingURL=parseNextInvalidImportError.js.map