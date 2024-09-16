import type { LoaderTree } from '../lib/app-dir-module';
import type { FlightRouterState, Segment } from './types';
import type { GetDynamicParamFromSegment } from './app-render';
export declare function addSearchParamsIfPageSegment(segment: Segment, searchParams: any): string | [string, string, "d" | "c" | "ci" | "oc" | "di"];
export declare function createFlightRouterStateFromLoaderTree([segment, parallelRoutes, { layout }]: LoaderTree, getDynamicParamFromSegment: GetDynamicParamFromSegment, searchParams: any, rootLayoutIncluded?: boolean): FlightRouterState;
