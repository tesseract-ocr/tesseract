/**
 * A `Promise.withResolvers` implementation that exposes the `resolve` and
 * `reject` functions on a `Promise`.
 *
 * @see https://tc39.es/proposal-promise-with-resolvers/
 */
export declare class DetachedPromise<T = any> {
    readonly resolve: (value: T | PromiseLike<T>) => void;
    readonly reject: (reason: any) => void;
    readonly promise: Promise<T>;
    constructor();
}
