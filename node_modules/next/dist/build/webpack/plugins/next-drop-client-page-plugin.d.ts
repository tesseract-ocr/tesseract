import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare const ampFirstEntryNamesMap: WeakMap<webpack.Compilation, string[]>;
export declare class DropClientPage implements webpack.WebpackPluginInstance {
    ampPages: Set<unknown>;
    apply(compiler: webpack.Compiler): void;
}
