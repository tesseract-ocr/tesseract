import type { Span } from '../../../trace';
import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare const spans: WeakMap<webpack.Compiler | webpack.Compilation, Span>;
export declare const webpackInvalidSpans: WeakMap<any, Span>;
export declare class ProfilingPlugin {
    compiler: any;
    runWebpackSpan: Span;
    rootDir: string;
    constructor({ runWebpackSpan, rootDir, }: {
        runWebpackSpan: Span;
        rootDir: string;
    });
    apply(compiler: any): void;
    traceHookPair(spanName: string | (() => string), startHook: any, stopHook: any, { parentSpan, attrs, onStart, onStop, }?: {
        parentSpan?: (...params: any[]) => Span;
        attrs?: any;
        onStart?: (span: Span, ...params: any[]) => void;
        onStop?: (span: Span, ...params: any[]) => void;
    }): void;
    traceTopLevelHooks(compiler: any): void;
    traceCompilationHooks(compiler: any): void;
}
