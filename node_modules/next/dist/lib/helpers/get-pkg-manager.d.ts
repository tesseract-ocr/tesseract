export type PackageManager = 'npm' | 'pnpm' | 'yarn';
export declare function getPkgManager(baseDir: string): PackageManager;
