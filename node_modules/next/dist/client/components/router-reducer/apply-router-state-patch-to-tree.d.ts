import type { FlightRouterState, FlightSegmentPath } from '../../../server/app-render/types';
/**
 * Apply the router state from the Flight response, but skip patching default segments.
 * Useful for patching the router cache when navigating, where we persist the existing default segment if there isn't a new one.
 * Creates a new router state tree.
 */
export declare function applyRouterStatePatchToTree(flightSegmentPath: FlightSegmentPath, flightRouterState: FlightRouterState, treePatch: FlightRouterState, path: string): FlightRouterState | null;
