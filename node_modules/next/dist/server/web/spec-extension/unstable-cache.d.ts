type Callback = (...args: any[]) => Promise<any>;
/**
 * This function allows you to cache the results of expensive operations, like database queries, and reuse them across multiple requests.
 *
 * Read more: [Next.js Docs: `unstable_cache`](https://nextjs.org/docs/app/api-reference/functions/unstable_cache)
 */
export declare function unstable_cache<T extends Callback>(cb: T, keyParts?: string[], options?: {
    /**
     * The revalidation interval in seconds.
     */
    revalidate?: number | false;
    tags?: string[];
}): T;
export {};
