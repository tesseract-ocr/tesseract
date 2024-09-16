declare const FNV_PRIMES: {
    readonly 32: bigint;
    readonly 64: bigint;
    readonly 128: bigint;
    readonly 256: bigint;
    readonly 512: bigint;
    readonly 1024: bigint;
};
export default function fnv1a(inputString: string, { size, seed, }?: {
    size?: keyof typeof FNV_PRIMES;
    seed?: number;
}): bigint;
export {};
