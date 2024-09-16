import type { COMPILER_INDEXES } from '../../shared/lib/constants';
import { type TelemetryPluginState } from '../webpack/plugins/telemetry-plugin';
import { NextBuildContext } from '../build-context';
import { type TraceEvent, type TraceState } from '../../trace';
import type { BuildTraceContext } from '../webpack/plugins/next-trace-entrypoints-plugin';
export declare function webpackBuildImpl(compilerName: keyof typeof COMPILER_INDEXES | null): Promise<{
    duration: number;
    pluginState: any;
    buildTraceContext?: BuildTraceContext;
    telemetryState?: TelemetryPluginState;
}>;
export declare function workerMain(workerData: {
    compilerName: keyof typeof COMPILER_INDEXES;
    buildContext: typeof NextBuildContext;
    traceState: TraceState;
}): Promise<Awaited<ReturnType<typeof webpackBuildImpl>> & {
    debugTraceEvents: TraceEvent[];
}>;
