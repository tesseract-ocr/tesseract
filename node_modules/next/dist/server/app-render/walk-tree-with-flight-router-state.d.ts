/// <reference types="react" />
import type { FlightDataPath, FlightRouterState } from './types';
import type { LoaderTree } from '../lib/app-dir-module';
import type { CreateSegmentPath, AppRenderContext } from './app-render';
/**
 * Use router state to decide at what common layout to render the page.
 * This can either be the common layout between two pages or a specific place to start rendering from using the "refetch" marker in the tree.
 */
export declare function walkTreeWithFlightRouterState({ createSegmentPath, loaderTreeToFilter, parentParams, isFirst, flightRouterState, parentRendered, rscPayloadHead, injectedCSS, injectedJS, injectedFontPreloadTags, rootLayoutIncluded, asNotFound, metadataOutlet, ctx, }: {
    createSegmentPath: CreateSegmentPath;
    loaderTreeToFilter: LoaderTree;
    parentParams: {
        [key: string]: string | string[];
    };
    isFirst: boolean;
    flightRouterState?: FlightRouterState;
    parentRendered?: boolean;
    rscPayloadHead: React.ReactNode;
    injectedCSS: Set<string>;
    injectedJS: Set<string>;
    injectedFontPreloadTags: Set<string>;
    rootLayoutIncluded: boolean;
    asNotFound?: boolean;
    metadataOutlet: React.ReactNode;
    ctx: AppRenderContext;
}): Promise<FlightDataPath[]>;
