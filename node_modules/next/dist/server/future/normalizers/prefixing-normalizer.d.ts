import type { Normalizer } from './normalizer';
export declare class PrefixingNormalizer implements Normalizer {
    private readonly prefix;
    constructor(...prefixes: ReadonlyArray<string>);
    normalize(pathname: string): string;
}
