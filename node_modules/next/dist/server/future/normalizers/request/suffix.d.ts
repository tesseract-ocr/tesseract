import type { Normalizer } from '../normalizer';
export declare class SuffixPathnameNormalizer implements Normalizer {
    private readonly suffix;
    constructor(suffix: string);
    match(pathname: string): boolean;
    normalize(pathname: string, matched?: boolean): string;
}
