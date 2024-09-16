import { PrefixingNormalizer } from '../../prefixing-normalizer';
export declare class PagesFilenameNormalizer extends PrefixingNormalizer {
    constructor(distDir: string);
    normalize(manifestFilename: string): string;
}
