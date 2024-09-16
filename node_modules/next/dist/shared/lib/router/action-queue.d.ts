import { type AppRouterState, type ReducerActions, type ReducerState } from '../../../client/components/router-reducer/router-reducer-types';
import type { ReduxDevToolsInstance } from '../../../client/components/use-reducer-with-devtools';
import React from 'react';
export type DispatchStatePromise = React.Dispatch<ReducerState>;
export type AppRouterActionQueue = {
    state: AppRouterState | null;
    devToolsInstance?: ReduxDevToolsInstance;
    dispatch: (payload: ReducerActions, setState: DispatchStatePromise) => void;
    action: (state: AppRouterState, action: ReducerActions) => ReducerState;
    pending: ActionQueueNode | null;
    needsRefresh?: boolean;
    last: ActionQueueNode | null;
};
export type ActionQueueNode = {
    payload: ReducerActions;
    next: ActionQueueNode | null;
    resolve: (value: ReducerState) => void;
    reject: (err: Error) => void;
    discarded?: boolean;
};
export declare const ActionQueueContext: React.Context<AppRouterActionQueue | null>;
export declare function createMutableActionQueue(): AppRouterActionQueue;
