import type { PackageManager } from './get-pkg-manager';
interface InstallArgs {
    /**
     * Indicate whether to install packages using npm, pnpm, or yarn.
     */
    packageManager: PackageManager;
    /**
     * Indicate whether there is an active internet connection.
     */
    isOnline: boolean;
    /**
     * Indicate whether the given dependencies are devDependencies.
     */
    devDependencies?: boolean;
}
/**
 * Spawn a package manager installation with either npm, pnpm, or yarn.
 *
 * @returns A Promise that resolves once the installation is finished.
 */
export declare function install(root: string, dependencies: string[], { packageManager, isOnline, devDependencies }: InstallArgs): Promise<void>;
export {};
