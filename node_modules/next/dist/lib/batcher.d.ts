import type { SchedulerFn } from './scheduler';
type CacheKeyFn<K, C extends string | number | null> = (key: K) => PromiseLike<C> | C;
type BatcherOptions<K, C extends string | number | null> = {
    cacheKeyFn?: CacheKeyFn<K, C>;
    schedulerFn?: SchedulerFn<void>;
};
type WorkFn<V, C> = (key: C, resolve: (value: V | PromiseLike<V>) => void) => Promise<V>;
/**
 * A wrapper for a function that will only allow one call to the function to
 * execute at a time.
 */
export declare class Batcher<K, V, C extends string | number | null> {
    private readonly cacheKeyFn?;
    /**
     * A function that will be called to schedule the wrapped function to be
     * executed. This defaults to a function that will execute the function
     * immediately.
     */
    private readonly schedulerFn;
    private readonly pending;
    protected constructor(cacheKeyFn?: CacheKeyFn<K, C> | undefined, 
    /**
     * A function that will be called to schedule the wrapped function to be
     * executed. This defaults to a function that will execute the function
     * immediately.
     */
    schedulerFn?: SchedulerFn<void>);
    /**
     * Creates a new instance of PendingWrapper. If the key extends a string or
     * number, the key will be used as the cache key. If the key is an object, a
     * cache key function must be provided.
     */
    static create<K extends string | number | null, V>(options?: BatcherOptions<K, K>): Batcher<K, V, K>;
    static create<K, V, C extends string | number | null>(options: BatcherOptions<K, C> & Required<Pick<BatcherOptions<K, C>, 'cacheKeyFn'>>): Batcher<K, V, C>;
    /**
     * Wraps a function in a promise that will be resolved or rejected only once
     * for a given key. This will allow multiple calls to the function to be
     * made, but only one will be executed at a time. The result of the first
     * call will be returned to all callers.
     *
     * @param key the key to use for the cache
     * @param fn the function to wrap
     * @returns a promise that resolves to the result of the function
     */
    batch(key: K, fn: WorkFn<V, C>): Promise<V>;
}
export {};
