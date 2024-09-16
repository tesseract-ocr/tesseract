import type { PrefetchAction, ReducerState, ReadonlyReducerState } from '../router-reducer-types';
import { PromiseQueue } from '../../promise-queue';
export declare const prefetchQueue: PromiseQueue;
export declare function prefetchReducer(state: ReadonlyReducerState, action: PrefetchAction): ReducerState;
