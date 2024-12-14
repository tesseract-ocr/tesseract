import type { webpack } from 'next/dist/compiled/webpack/webpack';
type Compiler = webpack.Compiler;
type WebpackPluginInstance = webpack.WebpackPluginInstance;
export declare class NextJsRequireCacheHotReloader implements WebpackPluginInstance {
    prevAssets: any;
    serverComponents: boolean;
    constructor(opts: {
        serverComponents: boolean;
    });
    apply(compiler: Compiler): void;
}
export {};
