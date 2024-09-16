import type tsModule from 'typescript/lib/tsserverlibrary';
declare const errorEntry: {
    getSemanticDiagnostics(source: tsModule.SourceFile, isClientEntry: boolean): tsModule.Diagnostic[];
};
export default errorEntry;
