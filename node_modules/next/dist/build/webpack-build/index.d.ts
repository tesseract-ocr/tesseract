import type { COMPILER_INDEXES } from '../../shared/lib/constants';
import type { BuildTraceContext } from '../webpack/plugins/next-trace-entrypoints-plugin';
declare const ORDERED_COMPILER_NAMES: (keyof typeof COMPILER_INDEXES)[];
declare function webpackBuildWithWorker(compilerNamesArg: typeof ORDERED_COMPILER_NAMES | null): Promise<{
    duration: number;
    buildTraceContext: BuildTraceContext;
}>;
export declare function webpackBuild(withWorker: boolean, compilerNames: typeof ORDERED_COMPILER_NAMES | null): ReturnType<typeof webpackBuildWithWorker>;
export {};
