export declare class BloomFilter {
    numItems: number;
    errorRate: number;
    numBits: number;
    numHashes: number;
    bitArray: number[];
    constructor(numItems: number, errorRate?: number);
    static from(items: string[], errorRate?: number): BloomFilter;
    export(): {
        numItems: number;
        errorRate: number;
        numBits: number;
        numHashes: number;
        bitArray: number[];
    };
    import(data: ReturnType<(typeof this)['export']>): void;
    add(item: string): void;
    contains(item: string): boolean;
    getHashValues(item: string): number[];
}
