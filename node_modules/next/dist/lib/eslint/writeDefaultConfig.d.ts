import type { ConfigAvailable } from './hasEslintConfiguration';
export declare function writeDefaultConfig(baseDir: string, { exists, emptyEslintrc, emptyPkgJsonConfig }: ConfigAvailable, selectedConfig: any, eslintrcFile: string | null, pkgJsonPath: string | null, packageJsonConfig: {
    eslintConfig: any;
} | null): Promise<void>;
