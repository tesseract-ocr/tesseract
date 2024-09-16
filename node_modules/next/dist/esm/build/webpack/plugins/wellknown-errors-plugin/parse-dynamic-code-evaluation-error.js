import { formatModuleTrace, getModuleTrace } from "./getModuleTrace";
import { SimpleWebpackError } from "./simpleWebpackError";
export function getDynamicCodeEvaluationError(message, module, compilation, compiler) {
    const { moduleTrace } = getModuleTrace(module, compilation, compiler);
    const { formattedModuleTrace, lastInternalFileName, invalidImportMessage } = formatModuleTrace(compiler, moduleTrace);
    return new SimpleWebpackError(lastInternalFileName, message + invalidImportMessage + "\n\nImport trace for requested module:\n" + formattedModuleTrace);
}

//# sourceMappingURL=parse-dynamic-code-evaluation-error.js.map