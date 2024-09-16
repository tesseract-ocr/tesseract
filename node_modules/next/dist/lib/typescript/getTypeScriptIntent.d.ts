export type TypeScriptIntent = {
    firstTimeSetup: boolean;
};
export declare function getTypeScriptIntent(baseDir: string, intentDirs: string[], tsconfigPath: string): Promise<TypeScriptIntent | false>;
