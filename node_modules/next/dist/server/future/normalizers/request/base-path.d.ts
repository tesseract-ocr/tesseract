import type { PathnameNormalizer } from './pathname-normalizer';
import { PrefixPathnameNormalizer } from './prefix';
export declare class BasePathPathnameNormalizer extends PrefixPathnameNormalizer implements PathnameNormalizer {
    constructor(basePath: string);
}
