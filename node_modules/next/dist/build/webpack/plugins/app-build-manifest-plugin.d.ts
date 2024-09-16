type Options = {
    dev: boolean;
};
export type AppBuildManifest = {
    pages: Record<string, string[]>;
};
export declare class AppBuildManifestPlugin {
    private readonly dev;
    constructor(options: Options);
    apply(compiler: any): void;
    private createAsset;
}
export {};
