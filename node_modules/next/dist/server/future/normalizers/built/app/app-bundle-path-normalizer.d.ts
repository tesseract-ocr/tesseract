import { Normalizers } from '../../normalizers';
import type { Normalizer } from '../../normalizer';
import { PrefixingNormalizer } from '../../prefixing-normalizer';
export declare class AppBundlePathNormalizer extends PrefixingNormalizer {
    constructor();
    normalize(page: string): string;
}
export declare class DevAppBundlePathNormalizer extends Normalizers {
    constructor(pageNormalizer: Normalizer);
    normalize(filename: string): string;
}
