import { webpack } from 'next/dist/compiled/webpack/webpack';
type CssMinimizerPluginOptions = {
    postcssOptions: {
        map: false | {
            prev?: string | false;
            inline: boolean;
            annotation: boolean;
        };
    };
};
export declare class CssMinimizerPlugin {
    __next_css_remove: boolean;
    private options;
    constructor(options: CssMinimizerPluginOptions);
    optimizeAsset(file: string, asset: any): Promise<import("webpack-sources1").RawSource | import("webpack-sources1").SourceMapSource>;
    apply(compiler: webpack.Compiler): void;
}
export {};
