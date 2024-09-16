import type tsModule from 'typescript/lib/tsserverlibrary';
declare const entry: {
    getCompletionsAtPosition(fileName: string, node: tsModule.FunctionDeclaration, position: number): tsModule.CompletionEntry[];
    getSemanticDiagnostics(fileName: string, source: tsModule.SourceFile, node: tsModule.FunctionDeclaration): tsModule.Diagnostic[];
};
export default entry;
