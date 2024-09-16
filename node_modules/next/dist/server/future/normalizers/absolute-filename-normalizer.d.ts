import type { PAGE_TYPES } from '../../../lib/page-types';
import type { Normalizer } from './normalizer';
/**
 * Normalizes a given filename so that it's relative to the provided directory.
 * It will also strip the extension (if provided) and the trailing `/index`.
 */
export declare class AbsoluteFilenameNormalizer implements Normalizer {
    private readonly dir;
    private readonly extensions;
    private readonly pagesType;
    /**
     *
     * @param dir the directory for which the files should be made relative to
     * @param extensions the extensions the file could have
     * @param keepIndex when `true` the trailing `/index` is _not_ removed
     */
    constructor(dir: string, extensions: ReadonlyArray<string>, pagesType: PAGE_TYPES);
    normalize(filename: string): string;
}
