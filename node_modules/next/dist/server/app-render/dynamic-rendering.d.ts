/**
 * The functions provided by this module are used to communicate certain properties
 * about the currently running code so that Next.js can make decisions on how to handle
 * the current execution in different rendering modes such as pre-rendering, resuming, and SSR.
 *
 * Today Next.js treats all code as potentially static. Certain APIs may only make sense when dynamically rendering.
 * Traditionally this meant deopting the entire render to dynamic however with PPR we can now deopt parts
 * of a React tree as dynamic while still keeping other parts static. There are really two different kinds of
 * Dynamic indications.
 *
 * The first is simply an intention to be dynamic. unstable_noStore is an example of this where
 * the currently executing code simply declares that the current scope is dynamic but if you use it
 * inside unstable_cache it can still be cached. This type of indication can be removed if we ever
 * make the default dynamic to begin with because the only way you would ever be static is inside
 * a cache scope which this indication does not affect.
 *
 * The second is an indication that a dynamic data source was read. This is a stronger form of dynamic
 * because it means that it is inappropriate to cache this at all. using a dynamic data source inside
 * unstable_cache should error. If you want to use some dynamic data inside unstable_cache you should
 * read that data outside the cache and pass it in as an argument to the cached function.
 */
import type { StaticGenerationStore } from '../../client/components/static-generation-async-storage.external';
type DynamicAccess = {
    /**
     * If debugging, this will contain the stack trace of where the dynamic access
     * occurred. This is used to provide more information to the user about why
     * their page is being rendered dynamically.
     */
    stack?: string;
    /**
     * The expression that was accessed dynamically.
     */
    expression: string;
};
export type PrerenderState = {
    /**
     * When true, stack information will also be tracked during dynamic access.
     */
    readonly isDebugSkeleton: boolean | undefined;
    /**
     * The dynamic accesses that occurred during the render.
     */
    readonly dynamicAccesses: DynamicAccess[];
};
export declare function createPrerenderState(isDebugSkeleton: boolean | undefined): PrerenderState;
/**
 * This function communicates that the current scope should be treated as dynamic.
 *
 * In most cases this function is a no-op but if called during
 * a PPR prerender it will postpone the current sub-tree.
 */
export declare function markCurrentScopeAsDynamic(store: StaticGenerationStore, expression: string): void;
/**
 * This function communicates that some dynamic data was read. This typically would refer to accessing
 * a Request specific data store such as cookies or headers. This function is not how end-users will
 * describe reading from dynamic data sources which are valid to cache and up to the author to make
 * a determination of when to do so.
 *
 * If we are inside a cache scope we error
 * Also during a PPR Prerender we postpone
 */
export declare function trackDynamicDataAccessed(store: StaticGenerationStore, expression: string): void;
/**
 * This component will call `React.postpone` that throws the postponed error.
 */
type PostponeProps = {
    reason: string;
    prerenderState: PrerenderState;
    pathname: string;
};
export declare function Postpone({ reason, prerenderState, pathname, }: PostponeProps): never;
export declare function trackDynamicFetch(store: StaticGenerationStore, expression: string): void;
export declare function usedDynamicAPIs(prerenderState: PrerenderState): boolean;
export declare function formatDynamicAPIAccesses(prerenderState: PrerenderState): string[];
/**
 * This is a bit of a hack to allow us to abort a render using a Postpone instance instead of an Error which changes React's
 * abort semantics slightly.
 */
export declare function createPostponedAbortSignal(reason: string): AbortSignal;
export {};
