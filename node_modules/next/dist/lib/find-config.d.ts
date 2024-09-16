type RecursivePartial<T> = {
    [P in keyof T]?: RecursivePartial<T[P]>;
};
export declare function findConfigPath(dir: string, key: string): Promise<string | undefined>;
export declare function findConfig<T>(directory: string, key: string, _returnFile?: boolean): Promise<RecursivePartial<T> | null>;
export {};
