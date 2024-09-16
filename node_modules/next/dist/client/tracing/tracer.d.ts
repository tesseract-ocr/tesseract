import type { MittEmitter } from '../../shared/lib/mitt';
export type SpanOptions = {
    startTime?: number;
    attributes?: Record<string, unknown>;
};
export type SpanState = {
    state: 'inprogress';
} | {
    state: 'ended';
    endTime: number;
};
interface ISpan {
    name: string;
    startTime: number;
    attributes: Record<string, unknown>;
    state: SpanState;
    end(endTime?: number): void;
}
declare class Span implements ISpan {
    name: string;
    startTime: number;
    onSpanEnd: (span: Span) => void;
    state: SpanState;
    attributes: Record<string, unknown>;
    constructor(name: string, options: SpanOptions, onSpanEnd: (span: Span) => void);
    end(endTime?: number): void;
}
declare class Tracer {
    _emitter: MittEmitter<string>;
    private handleSpanEnd;
    startSpan(name: string, options: SpanOptions): Span;
    onSpanEnd(cb: (span: ISpan) => void): () => void;
}
export type { ISpan as Span };
declare const _default: Tracer;
export default _default;
