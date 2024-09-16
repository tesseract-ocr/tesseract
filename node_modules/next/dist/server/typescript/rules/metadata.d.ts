import type tsModule from 'typescript/lib/tsserverlibrary';
declare const metadata: {
    filterCompletionsAtPosition(fileName: string, position: number, _options: any, prior: tsModule.WithMetadata<tsModule.CompletionInfo>): tsModule.WithMetadata<tsModule.CompletionInfo>;
    getSemanticDiagnosticsForExportVariableStatementInClientEntry(fileName: string, node: tsModule.VariableStatement | tsModule.FunctionDeclaration): {
        file: tsModule.SourceFile | undefined;
        category: tsModule.DiagnosticCategory;
        code: number;
        messageText: string;
        start: number;
        length: number;
    }[];
    getSemanticDiagnosticsForExportVariableStatement(fileName: string, node: tsModule.VariableStatement | tsModule.FunctionDeclaration): {
        file: tsModule.SourceFile | undefined;
        category: tsModule.DiagnosticCategory;
        code: number;
        messageText: string | tsModule.DiagnosticMessageChain;
        start: number | undefined;
        length: number | undefined;
    }[];
    getSemanticDiagnosticsForExportDeclarationInClientEntry(fileName: string, node: tsModule.ExportDeclaration): tsModule.Diagnostic[];
    getSemanticDiagnosticsForExportDeclaration(fileName: string, node: tsModule.ExportDeclaration): {
        file: tsModule.SourceFile | undefined;
        category: tsModule.DiagnosticCategory;
        code: number;
        messageText: string | tsModule.DiagnosticMessageChain;
        start: number | undefined;
        length: number | undefined;
    }[];
    getCompletionEntryDetails(fileName: string, position: number, entryName: string, formatOptions: tsModule.FormatCodeOptions, source: string, preferences: tsModule.UserPreferences, data: tsModule.CompletionEntryData): tsModule.CompletionEntryDetails | undefined;
    getQuickInfoAtPosition(fileName: string, position: number): tsModule.QuickInfo | undefined;
    getDefinitionAndBoundSpan(fileName: string, position: number): tsModule.DefinitionInfoAndBoundSpan | undefined;
};
export default metadata;
