type PackageJsonDependencies = {
    dependencies: Record<string, string>;
    devDependencies: Record<string, string>;
};
export declare function getDependencies({ cwd, }: {
    cwd: string;
}): Promise<PackageJsonDependencies>;
export declare function getPackageVersion({ cwd, name, }: {
    cwd: string;
    name: string;
}): Promise<string | null>;
export {};
