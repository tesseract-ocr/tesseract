/// <reference types="react" />
import type { FlightRouterState } from '../../server/app-render/types';
export declare function AppRouterAnnouncer({ tree }: {
    tree: FlightRouterState;
}): import("react").ReactPortal | null;
