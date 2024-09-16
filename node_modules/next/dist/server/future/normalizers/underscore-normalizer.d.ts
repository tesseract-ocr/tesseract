import type { Normalizer } from './normalizer';
/**
 * UnderscoreNormalizer replaces all instances of %5F with _.
 */
export declare class UnderscoreNormalizer implements Normalizer {
    normalize(pathname: string): string;
}
