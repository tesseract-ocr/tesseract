import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { Span } from '../trace';
export type CompilerResult = {
    errors: webpack.StatsError[];
    warnings: webpack.StatsError[];
    stats: webpack.Stats | undefined;
};
export declare function runCompiler(config: webpack.Configuration, { runWebpackSpan, inputFileSystem, }: {
    runWebpackSpan: Span;
    inputFileSystem?: webpack.Compiler['inputFileSystem'];
}): Promise<[
    result: CompilerResult,
    inputFileSystem?: webpack.Compiler['inputFileSystem']
]>;
