type Filter = (pathname: string) => boolean;
export type RecursiveReadDirOptions = {
    /**
     * Filter to ignore files with absolute pathnames, false to ignore.
     */
    pathnameFilter?: Filter;
    /**
     * Filter to ignore files and directories with absolute pathnames, false to
     * ignore.
     */
    ignoreFilter?: Filter;
    /**
     * Filter to ignore files and directories with the pathname part, false to
     * ignore.
     */
    ignorePartFilter?: Filter;
    /**
     * Whether to sort the results, true by default.
     */
    sortPathnames?: boolean;
    /**
     * Whether to return relative pathnames, true by default.
     */
    relativePathnames?: boolean;
};
/**
 * Recursively reads a directory and returns the list of pathnames.
 *
 * @param rootDirectory the directory to read
 * @param options options to control the behavior of the recursive read
 * @returns the list of pathnames
 */
export declare function recursiveReadDir(rootDirectory: string, options?: RecursiveReadDirOptions): Promise<string[]>;
export {};
