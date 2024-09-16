import type tsModule from 'typescript/lib/tsserverlibrary';
declare const serverLayer: {
    filterCompletionsAtPosition(entries: tsModule.CompletionEntry[]): tsModule.CompletionEntry[];
    hasDisallowedReactAPIDefinition(definitions: readonly tsModule.DefinitionInfo[]): boolean;
    getSemanticDiagnosticsForImportDeclaration(source: tsModule.SourceFile, node: tsModule.ImportDeclaration): tsModule.Diagnostic[];
};
export default serverLayer;
