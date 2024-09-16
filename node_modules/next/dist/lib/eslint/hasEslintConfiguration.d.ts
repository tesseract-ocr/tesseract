export type ConfigAvailable = {
    exists: boolean;
    emptyEslintrc?: boolean;
    emptyPkgJsonConfig?: boolean;
    firstTimeSetup?: true;
};
export declare function hasEslintConfiguration(eslintrcFile: string | null, packageJsonConfig: {
    eslintConfig: any;
} | null): Promise<ConfigAvailable>;
