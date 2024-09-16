/// <reference path="../../../../types/misc.d.ts" />
import type { SpanTypes } from './constants';
import type { ContextAPI, Span, SpanOptions, AttributeValue, TextMapGetter } from 'next/dist/compiled/@opentelemetry/api';
declare const SpanStatusCode: typeof import("next/dist/compiled/@opentelemetry/api").SpanStatusCode, SpanKind: typeof import("next/dist/compiled/@opentelemetry/api").SpanKind;
type TracerSpanOptions = Omit<SpanOptions, 'attributes'> & {
    parentSpan?: Span;
    spanName?: string;
    attributes?: Partial<Record<AttributeNames, AttributeValue | undefined>>;
    hideSpan?: boolean;
};
interface NextTracer {
    getContext(): ContextAPI;
    /**
     * Instruments a function by automatically creating a span activated on its
     * scope.
     *
     * The span will automatically be finished when one of these conditions is
     * met:
     *
     * * The function returns a promise, in which case the span will finish when
     * the promise is resolved or rejected.
     * * The function takes a callback as its second parameter, in which case the
     * span will finish when that callback is called.
     * * The function doesn't accept a callback and doesn't return a promise, in
     * which case the span will finish at the end of the function execution.
     *
     */
    trace<T>(type: SpanTypes, fn: (span?: Span, done?: (error?: Error) => any) => Promise<T>): Promise<T>;
    trace<T>(type: SpanTypes, fn: (span?: Span, done?: (error?: Error) => any) => T): T;
    trace<T>(type: SpanTypes, options: TracerSpanOptions, fn: (span?: Span, done?: (error?: Error) => any) => Promise<T>): Promise<T>;
    trace<T>(type: SpanTypes, options: TracerSpanOptions, fn: (span?: Span, done?: (error?: Error) => any) => T): T;
    /**
     * Wrap a function to automatically create a span activated on its
     * scope when it's called.
     *
     * The span will automatically be finished when one of these conditions is
     * met:
     *
     * * The function returns a promise, in which case the span will finish when
     * the promise is resolved or rejected.
     * * The function takes a callback as its last parameter, in which case the
     * span will finish when that callback is called.
     * * The function doesn't accept a callback and doesn't return a promise, in
     * which case the span will finish at the end of the function execution.
     */
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, fn: T): T;
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, options: TracerSpanOptions, fn: T): T;
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, options: (...args: any[]) => TracerSpanOptions, fn: T): T;
    /**
     * Starts and returns a new Span representing a logical unit of work.
     *
     * This method do NOT modify the current Context by default. In result, any inner span will not
     * automatically set its parent context to the span created by this method unless manually activate
     * context via `tracer.getContext().with`. `trace`, or `wrap` is generally recommended as it gracefully
     * handles context activation. (ref: https://github.com/open-telemetry/opentelemetry-js/issues/1923)
     */
    startSpan(type: SpanTypes): Span;
    startSpan(type: SpanTypes, options: TracerSpanOptions): Span;
    /**
     * Returns currently activated span if current context is in the scope of the span.
     * Returns undefined otherwise.
     */
    getActiveScopeSpan(): Span | undefined;
}
type NextAttributeNames = 'next.route' | 'next.page' | 'next.rsc' | 'next.segment' | 'next.span_name' | 'next.span_type' | 'next.clientComponentLoadCount';
type OTELAttributeNames = `http.${string}` | `net.${string}`;
type AttributeNames = NextAttributeNames | OTELAttributeNames;
declare class NextTracerImpl implements NextTracer {
    /**
     * Returns an instance to the trace with configured name.
     * Since wrap / trace can be defined in any place prior to actual trace subscriber initialization,
     * This should be lazily evaluated.
     */
    private getTracerInstance;
    getContext(): ContextAPI;
    getActiveScopeSpan(): Span | undefined;
    withPropagatedContext<T, C>(carrier: C, fn: () => T, getter?: TextMapGetter<C>): T;
    trace<T>(type: SpanTypes, fn: (span?: Span, done?: (error?: Error) => any) => Promise<T>): Promise<T>;
    trace<T>(type: SpanTypes, fn: (span?: Span, done?: (error?: Error) => any) => T): T;
    trace<T>(type: SpanTypes, options: TracerSpanOptions, fn: (span?: Span, done?: (error?: Error) => any) => Promise<T>): Promise<T>;
    trace<T>(type: SpanTypes, options: TracerSpanOptions, fn: (span?: Span, done?: (error?: Error) => any) => T): T;
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, fn: T): T;
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, options: TracerSpanOptions, fn: T): T;
    wrap<T = (...args: Array<any>) => any>(type: SpanTypes, options: (...args: any[]) => TracerSpanOptions, fn: T): T;
    startSpan(type: SpanTypes): Span;
    startSpan(type: SpanTypes, options: TracerSpanOptions): Span;
    private getSpanContext;
    getRootSpanAttributes(): Map<AttributeNames, AttributeValue | undefined> | undefined;
}
declare const getTracer: () => NextTracerImpl;
export { getTracer, SpanStatusCode, SpanKind };
export type { NextTracer, Span, SpanOptions, ContextAPI, TracerSpanOptions };
