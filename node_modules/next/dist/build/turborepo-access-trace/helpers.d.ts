import { TurborepoAccessTraceResult } from './result';
/**
 * Trace access to the filesystem (TODO), environment variables, and TCP addresses and
 * merge the results into the parent `TurborepoAccessTraceResult`.
 *
 * @param f the function to trace
 * @param parent the `TurborepoAccessTraceResult` to merge the results into
 * @returns the result of the function
 */
export declare function turborepoTraceAccess<T>(f: () => T | Promise<T>, parent: TurborepoAccessTraceResult): Promise<T> | T;
/**
 * Write the access trace to the trace file.
 *
 * @param distDir the directory to write the trace file to
 * @param traces an array of traces to merge and write to the trace file
 */
export declare function writeTurborepoAccessTraceResult({ distDir, traces, }: {
    distDir: string;
    traces: Array<TurborepoAccessTraceResult>;
}): Promise<void>;
