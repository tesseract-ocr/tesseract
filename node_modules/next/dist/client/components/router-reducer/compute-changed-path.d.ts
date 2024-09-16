import type { FlightRouterState } from '../../../server/app-render/types';
export declare function extractPathFromFlightRouterState(flightRouterState: FlightRouterState): string | undefined;
export declare function computeChangedPath(treeA: FlightRouterState, treeB: FlightRouterState): string | null;
