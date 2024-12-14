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
import type { WorkStore } from '../app-render/work-async-storage.external';
import type { WorkUnitStore, RequestStore, PrerenderStoreModern } from '../app-render/work-unit-async-storage.external';
export type DynamicAccess = {
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
export type DynamicTrackingState = {
    /**
     * When true, stack information will also be tracked during dynamic access.
     */
    readonly isDebugDynamicAccesses: boolean | undefined;
    /**
     * The dynamic accesses that occurred during the render.
     */
    readonly dynamicAccesses: Array<DynamicAccess>;
    syncDynamicExpression: undefined | string;
    syncDynamicErrorWithStack: null | Error;
    syncDynamicLogged?: boolean;
};
export type DynamicValidationState = {
    hasSuspendedDynamic: boolean;
    hasDynamicMetadata: boolean;
    hasDynamicViewport: boolean;
    hasSyncDynamicErrors: boolean;
    dynamicErrors: Array<Error>;
};
export declare function createDynamicTrackingState(isDebugDynamicAccesses: boolean | undefined): DynamicTrackingState;
export declare function createDynamicValidationState(): DynamicValidationState;
export declare function getFirstDynamicReason(trackingState: DynamicTrackingState): undefined | string;
/**
 * This function communicates that the current scope should be treated as dynamic.
 *
 * In most cases this function is a no-op but if called during
 * a PPR prerender it will postpone the current sub-tree and calling
 * it during a normal prerender will cause the entire prerender to abort
 */
export declare function markCurrentScopeAsDynamic(store: WorkStore, workUnitStore: undefined | Exclude<WorkUnitStore, PrerenderStoreModern>, expression: string): void;
/**
 * This function communicates that some dynamic path parameter was read. This
 * differs from the more general `trackDynamicDataAccessed` in that it is will
 * not error when `dynamic = "error"` is set.
 *
 * @param store The static generation store
 * @param expression The expression that was accessed dynamically
 */
export declare function trackFallbackParamAccessed(store: WorkStore, expression: string): void;
export declare function abortOnSynchronousPlatformIOAccess(route: string, expression: string, errorWithStack: Error, prerenderStore: PrerenderStoreModern): void;
export declare function trackSynchronousPlatformIOAccessInDev(requestStore: RequestStore): void;
export declare const trackSynchronousRequestDataAccessInDev: typeof trackSynchronousPlatformIOAccessInDev;
/**
 * This component will call `React.postpone` that throws the postponed error.
 */
type PostponeProps = {
    reason: string;
    route: string;
};
export declare function Postpone({ reason, route }: PostponeProps): never;
export declare function postponeWithTracking(route: string, expression: string, dynamicTracking: null | DynamicTrackingState): never;
export declare function isDynamicPostpone(err: unknown): boolean;
type DigestError = Error & {
    digest: string;
};
export declare function isPrerenderInterruptedError(error: unknown): error is DigestError;
export declare function accessedDynamicData(dynamicAccesses: Array<DynamicAccess>): boolean;
export declare function consumeDynamicAccess(serverDynamic: DynamicTrackingState, clientDynamic: DynamicTrackingState): DynamicTrackingState['dynamicAccesses'];
export declare function formatDynamicAPIAccesses(dynamicAccesses: Array<DynamicAccess>): string[];
/**
 * This is a bit of a hack to allow us to abort a render using a Postpone instance instead of an Error which changes React's
 * abort semantics slightly.
 */
export declare function createPostponedAbortSignal(reason: string): AbortSignal;
export declare function annotateDynamicAccess(expression: string, prerenderStore: PrerenderStoreModern): void;
export declare function useDynamicRouteParams(expression: string): void;
export declare function trackAllowedDynamicAccess(route: string, componentStack: string, dynamicValidation: DynamicValidationState, serverDynamic: DynamicTrackingState, clientDynamic: DynamicTrackingState): void;
export declare function throwIfDisallowedDynamic(route: string, dynamicValidation: DynamicValidationState, serverDynamic: DynamicTrackingState, clientDynamic: DynamicTrackingState): void;
export {};
