export type PackageManager = 'npm' | 'pnpm' | 'yarn';
interface InstallArgs {
    /**
     * Indicate whether to install packages using npm, pnpm or Yarn.
     */
    packageManager: PackageManager;
    /**
     * Indicate whether there is an active Internet connection.
     */
    isOnline: boolean;
    /**
     * Indicate whether the given dependencies are devDependencies.
     */
    devDependencies?: boolean;
}
/**
 * Spawn a package manager installation with either Yarn or NPM.
 *
 * @returns A Promise that resolves once the installation is finished.
 */
export declare function install(root: string, dependencies: string[] | null, { packageManager, isOnline, devDependencies }: InstallArgs): Promise<void>;
export {};
