import type { PrerenderManifest } from '../../../build';
import type { DeepReadonly } from '../../../shared/lib/deep-readonly';
import type { Revalidate } from '../revalidate';
/**
 * A shared cache of revalidate timings for routes. This cache is used so we
 * don't have to modify the prerender manifest when we want to update the
 * revalidate timings for a route.
 */
export declare class SharedRevalidateTimings {
    /**
     * The prerender manifest that contains the initial revalidate timings for
     * routes.
     */
    private readonly prerenderManifest;
    /**
     * The in-memory cache of revalidate timings for routes. This cache is
     * populated when the cache is updated with new timings.
     */
    private static readonly timings;
    constructor(
    /**
     * The prerender manifest that contains the initial revalidate timings for
     * routes.
     */
    prerenderManifest: DeepReadonly<Pick<PrerenderManifest, 'routes'>>);
    /**
     * Try to get the revalidate timings for a route. This will first try to get
     * the timings from the in-memory cache. If the timings are not present in the
     * in-memory cache, then the timings will be sourced from the prerender
     * manifest.
     *
     * @param route the route to get the revalidate timings for
     * @returns the revalidate timings for the route, or undefined if the timings
     *          are not present in the in-memory cache or the prerender manifest
     */
    get(route: string): Revalidate | undefined;
    /**
     * Set the revalidate timings for a route.
     *
     * @param route the route to set the revalidate timings for
     * @param revalidate the revalidate timings for the route
     */
    set(route: string, revalidate: Revalidate): void;
    /**
     * Clear the in-memory cache of revalidate timings for routes.
     */
    clear(): void;
}
