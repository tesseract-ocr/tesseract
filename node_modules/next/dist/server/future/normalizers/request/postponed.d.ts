import type { PathnameNormalizer } from './pathname-normalizer';
import { PrefixPathnameNormalizer } from './prefix';
export declare class PostponedPathnameNormalizer extends PrefixPathnameNormalizer implements PathnameNormalizer {
    constructor();
    normalize(pathname: string, matched?: boolean): string;
}
