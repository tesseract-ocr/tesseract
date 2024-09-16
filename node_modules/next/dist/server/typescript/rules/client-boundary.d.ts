import type tsModule from 'typescript/lib/tsserverlibrary';
declare const clientBoundary: {
    getSemanticDiagnosticsForExportVariableStatement(source: tsModule.SourceFile, node: tsModule.VariableStatement): tsModule.Diagnostic[];
    getSemanticDiagnosticsForFunctionExport(source: tsModule.SourceFile, node: tsModule.FunctionDeclaration | tsModule.ArrowFunction): tsModule.Diagnostic[];
};
export default clientBoundary;
