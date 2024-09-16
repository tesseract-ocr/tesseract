/**
 * Starts recording garbage collection events in the process and warn on long
 * running GCs. To disable, call `stopObservingGc`.
 */
export declare function startObservingGc(): void;
export declare function stopObservingGc(): void;
/**
 * Returns all recorded garbage collection events. This function will only
 * return information from when `startObservingGc` was enabled and before
 * `stopObservingGc` was called.
 */
export declare function getGcEvents(): PerformanceEntry[];
