import type { FileReader } from './file-reader';
/**
 * CachedFileReader will deduplicate requests made to the same folder structure
 * to scan for files.
 */
export declare class BatchedFileReader implements FileReader {
    private readonly reader;
    private batch?;
    constructor(reader: FileReader);
    private schedulePromise?;
    private schedule;
    private getOrCreateBatch;
    private load;
    read(dir: string): Promise<ReadonlyArray<string>>;
}
