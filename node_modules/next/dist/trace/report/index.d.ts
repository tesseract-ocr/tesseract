import type { TraceEvent } from '../types';
import type { Reporter } from './types';
declare class MultiReporter implements Reporter {
    private reporters;
    constructor(reporters: Reporter[]);
    flushAll(opts?: {
        end: boolean;
    }): Promise<void>;
    report(event: TraceEvent): void;
}
export declare const reporter: MultiReporter;
export {};
