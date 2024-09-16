// This module provides intellisense for all components that has the `"use client"` directive.
import { NEXT_TS_ERRORS } from "../constant";
import { getTs, getTypeChecker } from "../utils";
const clientBoundary = {
    getSemanticDiagnosticsForExportVariableStatement (source, node) {
        const ts = getTs();
        const diagnostics = [];
        if (ts.isVariableDeclarationList(node.declarationList)) {
            for (const declaration of node.declarationList.declarations){
                const initializer = declaration.initializer;
                if (initializer && ts.isArrowFunction(initializer)) {
                    diagnostics.push(...clientBoundary.getSemanticDiagnosticsForFunctionExport(source, initializer));
                }
            }
        }
        return diagnostics;
    },
    getSemanticDiagnosticsForFunctionExport (source, node) {
        var _node_parameters_, _node_parameters;
        const ts = getTs();
        const typeChecker = getTypeChecker();
        if (!typeChecker) return [];
        const diagnostics = [];
        const isErrorFile = /[\\/]error\.tsx?$/.test(source.fileName);
        const isGlobalErrorFile = /[\\/]global-error\.tsx?$/.test(source.fileName);
        const props = (_node_parameters = node.parameters) == null ? void 0 : (_node_parameters_ = _node_parameters[0]) == null ? void 0 : _node_parameters_.name;
        if (props && ts.isObjectBindingPattern(props)) {
            for (const prop of props.elements){
                var _type_symbol_getDeclarations, _type_symbol;
                const type = typeChecker.getTypeAtLocation(prop);
                const typeDeclarationNode = (_type_symbol = type.symbol) == null ? void 0 : (_type_symbol_getDeclarations = _type_symbol.getDeclarations()) == null ? void 0 : _type_symbol_getDeclarations[0];
                const propName = (prop.propertyName || prop.name).getText();
                if (typeDeclarationNode) {
                    if (// Show warning for not serializable props.
                    ts.isFunctionOrConstructorTypeNode(typeDeclarationNode) || ts.isClassDeclaration(typeDeclarationNode)) {
                        // There's a special case for the error file that the `reset` prop is allowed
                        // to be a function:
                        // https://github.com/vercel/next.js/issues/46573
                        if (!(isErrorFile || isGlobalErrorFile) || propName !== "reset") {
                            diagnostics.push({
                                file: source,
                                category: ts.DiagnosticCategory.Warning,
                                code: NEXT_TS_ERRORS.INVALID_CLIENT_ENTRY_PROP,
                                messageText: `Props must be serializable for components in the "use client" entry file, "${propName}" is invalid.`,
                                start: prop.getStart(),
                                length: prop.getWidth()
                            });
                        }
                    }
                }
            }
        }
        return diagnostics;
    }
};
export default clientBoundary;

//# sourceMappingURL=client-boundary.js.map