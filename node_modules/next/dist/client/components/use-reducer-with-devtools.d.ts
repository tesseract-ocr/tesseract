import type { Dispatch } from 'react';
import { type AppRouterState, type ReducerActions, type ReducerState } from './router-reducer/router-reducer-types';
export type ReduxDevtoolsSyncFn = (state: AppRouterState) => void;
declare global {
    interface Window {
        __REDUX_DEVTOOLS_EXTENSION__: any;
    }
}
export interface ReduxDevToolsInstance {
    send(action: any, state: any): void;
    init(initialState: any): void;
}
export declare function useUnwrapState(state: ReducerState): AppRouterState;
declare function useReducerWithReduxDevtoolsImpl(initialState: AppRouterState): [ReducerState, Dispatch<ReducerActions>, ReduxDevtoolsSyncFn];
export declare const useReducerWithReduxDevtools: typeof useReducerWithReduxDevtoolsImpl;
export {};
