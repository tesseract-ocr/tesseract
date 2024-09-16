import type { ReadonlyReducerState, ReducerState, FastRefreshAction } from '../router-reducer-types';
declare function fastRefreshReducerNoop(state: ReadonlyReducerState, _action: FastRefreshAction): ReducerState;
export declare const fastRefreshReducer: typeof fastRefreshReducerNoop;
export {};
