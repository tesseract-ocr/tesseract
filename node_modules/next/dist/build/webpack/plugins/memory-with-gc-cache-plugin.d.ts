import type { Compiler } from 'next/dist/compiled/webpack/webpack';
export declare class MemoryWithGcCachePlugin {
    /**
     * Maximum number of compilations to keep the cache entry around for when it's not used.
     * We keep the modules for a few more compilations so that if you comment out a package and bring it back it doesn't need a full compile again.
     */
    private maxGenerations;
    constructor({ maxGenerations }: {
        maxGenerations: number;
    });
    apply(compiler: Compiler): void;
}
