import type { FileReader } from './file-reader';
import type { RecursiveReadDirOptions } from '../../../../../../lib/recursive-readdir';
export type DefaultFileReaderOptions = Pick<RecursiveReadDirOptions, 'pathnameFilter' | 'ignoreFilter' | 'ignorePartFilter'>;
/**
 * Reads all the files in the directory and its subdirectories following any
 * symbolic links.
 */
export declare class DefaultFileReader implements FileReader {
    /**
     * Filter to ignore files with absolute pathnames. If undefined, no files are
     * ignored.
     */
    private readonly options;
    /**
     * Creates a new file reader.
     *
     * @param pathnameFilter filter to ignore files with absolute pathnames, false to ignore
     * @param ignoreFilter filter to ignore files and directories with absolute pathnames, false to ignore
     * @param ignorePartFilter filter to ignore files and directories with the pathname part, false to ignore
     */
    constructor(options: Readonly<DefaultFileReaderOptions>);
    /**
     * Reads all the files in the directory and its subdirectories following any
     * symbolic links.
     *
     * @param dir the directory to read
     * @returns a promise that resolves to the list of files
     */
    read(dir: string): Promise<ReadonlyArray<string>>;
}
