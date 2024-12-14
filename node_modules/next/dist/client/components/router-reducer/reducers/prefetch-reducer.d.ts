import type { PrefetchAction, ReducerState, ReadonlyReducerState } from '../router-reducer-types';
import { PromiseQueue } from '../../promise-queue';
export declare const prefetchQueue: PromiseQueue;
export declare const prefetchReducer: typeof identityReducerWhenSegmentCacheIsEnabled | typeof prefetchReducerImpl;
declare function identityReducerWhenSegmentCacheIsEnabled<T>(state: T): T;
declare function prefetchReducerImpl(state: ReadonlyReducerState, action: PrefetchAction): ReducerState;
export {};
