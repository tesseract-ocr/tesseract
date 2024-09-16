import { type Span } from '../../trace';
interface MemoryUsage {
    'memory.rss': number;
    'memory.heapUsed': number;
    'memory.heapTotal': number;
    'memory.heapMax': number;
}
/**
 * Begins a timer that will record memory usage periodically to understand
 * memory usage across the lifetime of the process.
 */
export declare function startPeriodicMemoryUsageTracing(): void;
export declare function stopPeriodicMemoryUsageTracing(): void;
/**
 * Returns the list of all recorded memory usage snapshots from the process.
 */
export declare function getAllMemoryUsageSpans(): MemoryUsage[];
/**
 * Records a snapshot of memory usage at this moment in time to the .next/trace
 * file.
 */
export declare function traceMemoryUsage(description: string, parentSpan?: Span | undefined): void;
export {};
