export type ScheduledFn<T = void> = () => T | PromiseLike<T>;
export type SchedulerFn<T = void> = (cb: ScheduledFn<T>) => void;
/**
 * Schedules a function to be called on the next tick after the other promises
 * have been resolved.
 *
 * @param cb the function to schedule
 */
export declare const scheduleOnNextTick: <T = void>(cb: ScheduledFn<T>) => void;
/**
 * Schedules a function to be called using `setImmediate` or `setTimeout` if
 * `setImmediate` is not available (like in the Edge runtime).
 *
 * @param cb the function to schedule
 */
export declare const scheduleImmediate: <T = void>(cb: ScheduledFn<T>) => void;
/**
 * returns a promise than resolves in a future task. There is no guarantee that the task it resolves in
 * will be the next task but if you await it you can at least be sure that the current task is over and
 * most usefully that the entire microtask queue of the current task has been emptied.
 */
export declare function atLeastOneTask(): Promise<void>;
