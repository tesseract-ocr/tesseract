import type { Compiler } from 'webpack';
export declare class CssChunkingPlugin {
    private strict;
    constructor(strict: boolean);
    apply(compiler: Compiler): void;
}
