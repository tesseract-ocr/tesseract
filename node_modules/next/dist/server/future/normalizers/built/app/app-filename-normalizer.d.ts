import { PrefixingNormalizer } from '../../prefixing-normalizer';
export declare class AppFilenameNormalizer extends PrefixingNormalizer {
    constructor(distDir: string);
    normalize(manifestFilename: string): string;
}
