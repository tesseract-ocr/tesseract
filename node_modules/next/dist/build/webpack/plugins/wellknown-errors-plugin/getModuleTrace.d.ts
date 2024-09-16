import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare function formatModuleTrace(compiler: webpack.Compiler, moduleTrace: any[]): {
    lastInternalFileName: string;
    invalidImportMessage: string;
    formattedModuleTrace: string;
};
export declare function getModuleTrace(module: any, compilation: webpack.Compilation, compiler: webpack.Compiler): {
    moduleTrace: any[];
    isPagesDir: boolean;
};
