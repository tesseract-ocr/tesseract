import type tsModule from 'typescript/lib/tsserverlibrary';
type TypeScript = typeof import('typescript/lib/tsserverlibrary');
export declare function log(message: string): void;
export declare function init(opts: {
    ts: TypeScript;
    info: tsModule.server.PluginCreateInfo;
}): void;
export declare function getTs(): typeof tsModule;
export declare function getInfo(): tsModule.server.PluginCreateInfo;
export declare function getTypeChecker(): tsModule.TypeChecker | undefined;
export declare function getSource(fileName: string): tsModule.SourceFile | undefined;
export declare function removeStringQuotes(str: string): string;
export declare const isPositionInsideNode: (position: number, node: tsModule.Node) => boolean;
export declare const isDefaultFunctionExport: (node: tsModule.Node) => node is tsModule.FunctionDeclaration;
export declare const isInsideApp: (filePath: string) => boolean;
export declare const isAppEntryFile: (filePath: string) => boolean;
export declare const isPageFile: (filePath: string) => boolean;
export declare function getEntryInfo(fileName: string, throwOnInvalidDirective?: boolean): {
    client: boolean;
    server: boolean;
};
export {};
