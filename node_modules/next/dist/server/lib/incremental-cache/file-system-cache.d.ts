import type { CacheHandler, CacheHandlerContext, CacheHandlerValue } from './';
import type { CacheFs } from '../../../shared/lib/utils';
type FileSystemCacheContext = Omit<CacheHandlerContext, 'fs' | 'serverDistDir'> & {
    fs: CacheFs;
    serverDistDir: string;
};
export default class FileSystemCache implements CacheHandler {
    private fs;
    private flushToDisk?;
    private serverDistDir;
    private revalidatedTags;
    private debug;
    constructor(ctx: FileSystemCacheContext);
    resetRequestCache(): void;
    revalidateTag(...args: Parameters<CacheHandler['revalidateTag']>): Promise<void>;
    get(...args: Parameters<CacheHandler['get']>): Promise<CacheHandlerValue | null>;
    set(...args: Parameters<CacheHandler['set']>): Promise<void>;
    private getFilePath;
}
export {};
