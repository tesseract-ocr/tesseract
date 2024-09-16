// This module provides intellisense for all exports from `"use server"` directive.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _constant = require("../constant");
const _utils = require("../utils");
// Check if the type is `Promise<T>`.
function isPromiseType(type, typeChecker) {
    const typeReferenceType = type;
    if (!typeReferenceType.target) return false;
    // target should be Promise or Promise<...>
    if (!/^Promise(<.+>)?$/.test(typeChecker.typeToString(typeReferenceType.target))) {
        return false;
    }
    return true;
}
function isFunctionReturningPromise(node, typeChecker, ts) {
    const type = typeChecker.getTypeAtLocation(node);
    const signatures = typeChecker.getSignaturesOfType(type, ts.SignatureKind.Call);
    let isPromise = true;
    if (signatures.length) {
        for (const signature of signatures){
            const returnType = signature.getReturnType();
            if (returnType.isUnion()) {
                for (const t of returnType.types){
                    if (!isPromiseType(t, typeChecker)) {
                        isPromise = false;
                        break;
                    }
                }
            } else {
                isPromise = isPromiseType(returnType, typeChecker);
            }
        }
    } else {
        isPromise = false;
    }
    return isPromise;
}
const serverBoundary = {
    getSemanticDiagnosticsForExportDeclaration (source, node) {
        const ts = (0, _utils.getTs)();
        const typeChecker = (0, _utils.getTypeChecker)();
        if (!typeChecker) return [];
        const diagnostics = [];
        const exportClause = node.exportClause;
        if (exportClause && ts.isNamedExports(exportClause)) {
            for (const e of exportClause.elements){
                if (!isFunctionReturningPromise(e, typeChecker, ts)) {
                    diagnostics.push({
                        file: source,
                        category: ts.DiagnosticCategory.Error,
                        code: _constant.NEXT_TS_ERRORS.INVALID_SERVER_ENTRY_RETURN,
                        messageText: `The "use server" file can only export async functions.`,
                        start: e.getStart(),
                        length: e.getWidth()
                    });
                }
            }
        }
        return diagnostics;
    },
    getSemanticDiagnosticsForExportVariableStatement (source, node) {
        const ts = (0, _utils.getTs)();
        const diagnostics = [];
        if (ts.isVariableDeclarationList(node.declarationList)) {
            for (const declaration of node.declarationList.declarations){
                const initializer = declaration.initializer;
                if (initializer && (ts.isArrowFunction(initializer) || ts.isFunctionDeclaration(initializer) || ts.isFunctionExpression(initializer) || ts.isCallExpression(initializer) || ts.isIdentifier(initializer))) {
                    diagnostics.push(...serverBoundary.getSemanticDiagnosticsForFunctionExport(source, initializer));
                } else {
                    diagnostics.push({
                        file: source,
                        category: ts.DiagnosticCategory.Error,
                        code: _constant.NEXT_TS_ERRORS.INVALID_SERVER_ENTRY_RETURN,
                        messageText: `The "use server" file can only export async functions.`,
                        start: declaration.getStart(),
                        length: declaration.getWidth()
                    });
                }
            }
        }
        return diagnostics;
    },
    getSemanticDiagnosticsForFunctionExport (source, node) {
        const ts = (0, _utils.getTs)();
        const typeChecker = (0, _utils.getTypeChecker)();
        if (!typeChecker) return [];
        const diagnostics = [];
        if (!isFunctionReturningPromise(node, typeChecker, ts)) {
            diagnostics.push({
                file: source,
                category: ts.DiagnosticCategory.Error,
                code: _constant.NEXT_TS_ERRORS.INVALID_SERVER_ENTRY_RETURN,
                messageText: `The "use server" file can only export async functions. Add "async" to the function declaration or return a Promise.`,
                start: node.getStart(),
                length: node.getWidth()
            });
        }
        return diagnostics;
    }
};
const _default = serverBoundary;

//# sourceMappingURL=server-boundary.js.map