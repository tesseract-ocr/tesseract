import type { BuildTraceContext } from '../webpack/plugins/next-trace-entrypoints-plugin';
declare const ORDERED_COMPILER_NAMES: import("../../shared/lib/constants").CompilerNameValues[];
declare function webpackBuildWithWorker(compilerNamesArg: typeof ORDERED_COMPILER_NAMES | null): Promise<{
    duration: number;
    buildTraceContext: BuildTraceContext;
}>;
export declare function webpackBuild(withWorker: boolean, compilerNames: typeof ORDERED_COMPILER_NAMES | null): ReturnType<typeof webpackBuildWithWorker>;
export {};
