import type { PageExtensions } from '../../build/page-extensions-type';
/**
 * Finds a page file with the given parameters. If the page is duplicated with
 * multiple extensions it will throw, otherwise it will return the *relative*
 * path to the page file or null if it is not found.
 *
 * @param pagesDir Absolute path to the pages folder with trailing `/pages`.
 * @param normalizedPagePath The page normalized (it will be denormalized).
 * @param pageExtensions Array of page extensions.
 */
export declare function findPageFile(pagesDir: string, normalizedPagePath: string, pageExtensions: PageExtensions, isAppDir: boolean): Promise<string | null>;
/**
 *
 * createValidFileMatcher receives configured page extensions and return helpers to determine:
 * `isLayoutsLeafPage`: if a file is a valid page file or routes file under app directory
 * `isTrackedFiles`: if it's a tracked file for webpack watcher
 *
 */
export declare function createValidFileMatcher(pageExtensions: PageExtensions, appDirPath: string | undefined): {
    isPageFile: (filePath: string) => boolean;
    isAppRouterPage: (filePath: string) => boolean;
    isMetadataFile: (filePath: string) => boolean;
    isRootNotFound: (filePath: string) => boolean;
};
