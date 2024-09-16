import type { FlightRouterState } from '../../../server/app-render/types';
import type { ReadonlyReducerState, ReducerActions } from './router-reducer-types';
/**
 * Handles the case where the client router attempted to patch the tree but, due to a mismatch, the patch failed.
 * This will perform an MPA navigation to return the router to a valid state.
 */
export declare function handleSegmentMismatch(state: ReadonlyReducerState, action: ReducerActions, treePatch: FlightRouterState): import("./router-reducer-types").ReducerState;
