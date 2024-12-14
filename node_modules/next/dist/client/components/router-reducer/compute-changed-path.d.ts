import type { FlightRouterState } from '../../../server/app-render/types';
import type { Params } from '../../../server/request/params';
export declare function extractPathFromFlightRouterState(flightRouterState: FlightRouterState): string | undefined;
export declare function computeChangedPath(treeA: FlightRouterState, treeB: FlightRouterState): string | null;
/**
 * Recursively extracts dynamic parameters from FlightRouterState.
 */
export declare function getSelectedParams(currentTree: FlightRouterState, params?: Params): Params;
