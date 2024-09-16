import type { Normalizer } from '../normalizer';
export declare class PrefixPathnameNormalizer implements Normalizer {
    private readonly prefix;
    constructor(prefix: string);
    match(pathname: string): boolean;
    normalize(pathname: string, matched?: boolean): string;
}
