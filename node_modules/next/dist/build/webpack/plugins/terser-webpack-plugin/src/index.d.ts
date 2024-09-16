export declare class TerserPlugin {
    options: any;
    constructor(options?: any);
    optimize(compiler: any, compilation: any, assets: any, optimizeOptions: any, cache: any, { SourceMapSource, RawSource }: any): Promise<void>;
    apply(compiler: any): void;
}
