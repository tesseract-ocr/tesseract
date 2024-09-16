export type Dependencies = {
    resolved: Map<string, string>;
};
export declare function installDependencies(baseDir: string, deps: any, dev?: boolean): Promise<void>;
