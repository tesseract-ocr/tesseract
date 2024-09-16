import { type Mutable, type NavigateAction, type ReadonlyReducerState, type ReducerState } from '../router-reducer-types';
export declare function handleExternalUrl(state: ReadonlyReducerState, mutable: Mutable, url: string, pendingPush: boolean): ReducerState;
export declare const navigateReducer: typeof navigateReducer_PPR;
declare function navigateReducer_PPR(state: ReadonlyReducerState, action: NavigateAction): ReducerState;
export {};
