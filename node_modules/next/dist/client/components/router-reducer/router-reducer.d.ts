import type { ReducerActions, ReducerState, ReadonlyReducerState } from './router-reducer-types';
declare function serverReducer(state: ReadonlyReducerState, _action: ReducerActions): ReducerState;
export declare const reducer: typeof serverReducer;
export {};
