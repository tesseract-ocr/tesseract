import type tsModule from 'typescript/lib/tsserverlibrary';
declare const serverBoundary: {
    getSemanticDiagnosticsForExportDeclaration(source: tsModule.SourceFile, node: tsModule.ExportDeclaration): tsModule.Diagnostic[];
    getSemanticDiagnosticsForExportVariableStatement(source: tsModule.SourceFile, node: tsModule.VariableStatement): tsModule.Diagnostic[];
    getSemanticDiagnosticsForFunctionExport(source: tsModule.SourceFile, node: tsModule.FunctionDeclaration | tsModule.ArrowFunction | tsModule.FunctionExpression | tsModule.CallExpression | tsModule.Identifier): tsModule.Diagnostic[];
};
export default serverBoundary;
