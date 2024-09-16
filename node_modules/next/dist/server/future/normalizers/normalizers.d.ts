import type { Normalizer } from './normalizer';
/**
 * Normalizers combines many normalizers into a single normalizer interface that
 * will normalize the inputted pathname with each normalizer in order.
 */
export declare class Normalizers implements Normalizer {
    private readonly normalizers;
    constructor(normalizers?: Array<Normalizer>);
    push(normalizer: Normalizer): void;
    normalize(pathname: string): string;
}
