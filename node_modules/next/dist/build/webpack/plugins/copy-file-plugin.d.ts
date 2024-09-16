import { webpack } from 'next/dist/compiled/webpack/webpack';
export declare class CopyFilePlugin {
    private filePath;
    private name;
    private cacheKey;
    private info?;
    constructor({ filePath, cacheKey, name, info, }: {
        filePath: string;
        cacheKey: string;
        name: string;
        minimize: boolean;
        info?: object;
    });
    apply(compiler: webpack.Compiler): void;
}
