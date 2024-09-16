export type Env = {
    [key: string]: string | undefined;
};
export type LoadedEnvFiles = Array<{
    path: string;
    contents: string;
}>;
export declare let initialEnv: Env | undefined;
export declare function updateInitialEnv(newEnv: Env): void;
type Log = {
    info: (...args: any[]) => void;
    error: (...args: any[]) => void;
};
export declare function processEnv(loadedEnvFiles: LoadedEnvFiles, dir?: string, log?: Log, forceReload?: boolean, onReload?: (envFilePath: string) => void): Env;
export declare function resetEnv(): void;
export declare function loadEnvConfig(dir: string, dev?: boolean, log?: Log, forceReload?: boolean, onReload?: (envFilePath: string) => void): {
    combinedEnv: Env;
    loadedEnvFiles: LoadedEnvFiles;
};
export {};
