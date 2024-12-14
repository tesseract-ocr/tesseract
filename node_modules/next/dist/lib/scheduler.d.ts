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
/**
 * This utility function is extracted to make it easier to find places where we are doing
 * specific timing tricks to try to schedule work after React has rendered. This is especially
 * important at the moment because Next.js uses the edge builds of React which use setTimeout to
 * schedule work when you might expect that something like setImmediate would do the trick.
 *
 * Long term we should switch to the node versions of React rendering when possible and then
 * update this to use setImmediate rather than setTimeout
 */
export declare function waitAtLeastOneReactRenderTask(): Promise<void>;
