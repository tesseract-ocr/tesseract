import type { PathnameNormalizer } from './pathname-normalizer';
export declare class NextDataPathnameNormalizer implements PathnameNormalizer {
    private readonly prefix;
    private readonly suffix;
    constructor(buildID: string);
    match(pathname: string): boolean;
    normalize(pathname: string, matched?: boolean): string;
}
