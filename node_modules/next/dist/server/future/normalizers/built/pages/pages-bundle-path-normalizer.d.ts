import type { Normalizer } from '../../normalizer';
import { Normalizers } from '../../normalizers';
export declare class PagesBundlePathNormalizer extends Normalizers {
    constructor();
    normalize(page: string): string;
}
export declare class DevPagesBundlePathNormalizer extends Normalizers {
    constructor(pagesNormalizer: Normalizer);
    normalize(filename: string): string;
}
