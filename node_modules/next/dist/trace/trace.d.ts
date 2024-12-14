import type { SpanId, TraceEvent, TraceState } from './types';
export declare enum SpanStatus {
    Started = "started",
    Stopped = "stopped"
}
interface Attributes {
    [key: string]: string;
}
export declare class Span {
    private name;
    private id;
    private parentId?;
    private attrs;
    private status;
    private now;
    private _start;
    constructor({ name, parentId, attrs, startTime, }: {
        name: string;
        parentId?: SpanId;
        startTime?: bigint;
        attrs?: Attributes;
    });
    stop(stopTime?: bigint): void;
    traceChild(name: string, attrs?: Attributes): Span;
    manualTraceChild(name: string, startTime?: bigint, stopTime?: bigint, attrs?: Attributes): void;
    getId(): number;
    setAttribute(key: string, value: string): void;
    traceFn<T>(fn: (span: Span) => T): T;
    traceAsyncFn<T>(fn: (span: Span) => T | Promise<T>): Promise<T>;
}
export declare const trace: (name: string, parentId?: SpanId, attrs?: {
    [key: string]: string;
}) => Span;
export declare const flushAllTraces: (opts?: {
    end: boolean;
}) => Promise<void>;
export declare const exportTraceState: () => TraceState;
export declare const initializeTraceState: (state: TraceState) => void;
export declare function getTraceEvents(): TraceEvent[];
export declare function recordTraceEvents(events: TraceEvent[]): void;
export declare const clearTraceEvents: () => never[];
export {};
