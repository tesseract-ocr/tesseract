import type { LoaderTree } from '../lib/app-dir-module';
import type { FlightRouterState } from './types';
import type { GetDynamicParamFromSegment } from './app-render';
export declare function createFlightRouterStateFromLoaderTree([segment, parallelRoutes, { layout }]: LoaderTree, getDynamicParamFromSegment: GetDynamicParamFromSegment, searchParams: any, rootLayoutIncluded?: boolean): FlightRouterState;
