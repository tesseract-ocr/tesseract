import type { TraceEvent } from '../types';
declare const _default: {
    flushAll: () => void;
    report: ({ name, duration }: TraceEvent) => void;
};
export default _default;
