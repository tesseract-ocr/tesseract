import type { TraceEvent } from '../types';
declare const localEndpoint: {
    serviceName: string;
    ipv4: string;
    port: number;
};
type Event = TraceEvent & {
    localEndpoint?: typeof localEndpoint;
};
export declare function batcher(reportEvents: (evts: Event[]) => Promise<void>): {
    flushAll: () => Promise<void>;
    report: (event: Event) => void;
};
declare const _default: {
    flushAll: (opts?: {
        end: boolean;
    }) => Promise<void | undefined> | undefined;
    report: (event: TraceEvent) => void;
};
export default _default;
