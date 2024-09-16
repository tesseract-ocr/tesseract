import type { Compiler } from 'webpack';
export declare class LightningCssMinifyPlugin {
    private readonly options;
    private transform;
    constructor(opts?: any);
    apply(compiler: Compiler): void;
    private transformAssets;
}
