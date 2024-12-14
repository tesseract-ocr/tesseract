// This module provides intellisense for all components that has the `"use client"` directive.
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
const clientBoundary = {
    getSemanticDiagnosticsForExportVariableStatement (source, node) {
        const ts = (0, _utils.getTs)();
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
        const ts = (0, _utils.getTs)();
        const typeChecker = (0, _utils.getTypeChecker)();
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
                    if (ts.isFunctionTypeNode(typeDeclarationNode)) {
                        // By convention, props named "action" can accept functions since we
                        // assume these are Server Actions. Structurally, there's no
                        // difference between a Server Action and a normal function until
                        // TypeScript exposes directives in the type of a function. This
                        // will miss accidentally passing normal functions but a false
                        // negative is better than a false positive given how frequent the
                        // false-positive would be.
                        const maybeServerAction = propName === 'action' || /.+Action$/.test(propName);
                        // There's a special case for the error file that the `reset` prop
                        // is allowed to be a function:
                        // https://github.com/vercel/next.js/issues/46573
                        const isErrorReset = (isErrorFile || isGlobalErrorFile) && propName === 'reset';
                        if (!maybeServerAction && !isErrorReset) {
                            diagnostics.push({
                                file: source,
                                category: ts.DiagnosticCategory.Warning,
                                code: _constant.NEXT_TS_ERRORS.INVALID_CLIENT_ENTRY_PROP,
                                messageText: `Props must be serializable for components in the "use client" entry file. ` + `"${propName}" is a function that's not a Server Action. ` + `Rename "${propName}" either to "action" or have its name end with "Action" e.g. "${propName}Action" to indicate it is a Server Action.`,
                                start: prop.getStart(),
                                length: prop.getWidth()
                            });
                        }
                    } else if (// Show warning for not serializable props.
                    ts.isConstructorTypeNode(typeDeclarationNode) || ts.isClassDeclaration(typeDeclarationNode)) {
                        diagnostics.push({
                            file: source,
                            category: ts.DiagnosticCategory.Warning,
                            code: _constant.NEXT_TS_ERRORS.INVALID_CLIENT_ENTRY_PROP,
                            messageText: `Props must be serializable for components in the "use client" entry file, "${propName}" is invalid.`,
                            start: prop.getStart(),
                            length: prop.getWidth()
                        });
                    }
                }
            }
        }
        return diagnostics;
    }
};
const _default = clientBoundary;

//# sourceMappingURL=client-boundary.js.map