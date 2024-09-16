/// <reference types="react" />
/// <reference types="node" />
import type { LoadComponentsReturnType } from '../load-components';
import type { ServerRuntime, SizeLimit } from '../../../types';
import type { NextConfigComplete } from '../../server/config-shared';
import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { NextFontManifest } from '../../build/webpack/plugins/next-font-manifest-plugin';
import type { ParsedUrlQuery } from 'querystring';
import type { AppPageModule } from '../future/route-modules/app-page/module';
import type { SwrDelta } from '../lib/revalidate';
import type { LoadingModuleData } from '../../shared/lib/app-router-context.shared-runtime';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
import s from 'next/dist/compiled/superstruct';
export type DynamicParamTypes = 'catchall' | 'catchall-intercepted' | 'optional-catchall' | 'dynamic' | 'dynamic-intercepted';
declare const dynamicParamTypesSchema: s.Struct<"d" | "c" | "ci" | "oc" | "di", {
    d: "d";
    c: "c";
    ci: "ci";
    oc: "oc";
    di: "di";
}>;
export type DynamicParamTypesShort = s.Infer<typeof dynamicParamTypesSchema>;
declare const segmentSchema: s.Struct<string | [string, string, "d" | "c" | "ci" | "oc" | "di"], null>;
export type Segment = s.Infer<typeof segmentSchema>;
export declare const flightRouterStateSchema: s.Describe<any>;
/**
 * Router state
 */
export type FlightRouterState = [
    segment: Segment,
    parallelRoutes: {
        [parallelRouterKey: string]: FlightRouterState;
    },
    url?: string | null,
    refresh?: 'refetch' | 'refresh' | null,
    isRootLayout?: boolean
];
/**
 * Individual Flight response path
 */
export type FlightSegmentPath = any[] | [
    segment: Segment,
    parallelRouterKey: string,
    segment: Segment,
    parallelRouterKey: string,
    segment: Segment,
    parallelRouterKey: string
];
/**
 * Represents a tree of segments and the Flight data (i.e. React nodes) that
 * correspond to each one. The tree is isomorphic to the FlightRouterState;
 * however in the future we want to be able to fetch arbitrary partial segments
 * without having to fetch all its children. So this response format will
 * likely change.
 */
export type CacheNodeSeedData = [
    segment: Segment,
    parallelRoutes: {
        [parallelRouterKey: string]: CacheNodeSeedData | null;
    },
    node: React.ReactNode | null,
    loading: LoadingModuleData
];
export type FlightDataPath = any[] | [
    ...FlightSegmentPath[],
    Segment,
    FlightRouterState,
    CacheNodeSeedData,
    // Can be null during prefetch if there's no loading component
    React.ReactNode | null
];
/**
 * The Flight response data
 */
export type FlightData = Array<FlightDataPath> | string;
export type ActionResult = Promise<any>;
export type NextFlightResponse = [buildId: string, flightData: FlightData];
export type ActionFlightResponse = [ActionResult, [buildId: string, flightData: FlightData | null]] | NextFlightResponse;
export interface RenderOptsPartial {
    err?: Error | null;
    dev?: boolean;
    buildId: string;
    basePath: string;
    trailingSlash: boolean;
    clientReferenceManifest?: DeepReadonly<ClientReferenceManifest>;
    supportsDynamicHTML: boolean;
    runtime?: ServerRuntime;
    serverComponents?: boolean;
    enableTainting?: boolean;
    assetPrefix?: string;
    crossOrigin?: '' | 'anonymous' | 'use-credentials' | undefined;
    nextFontManifest?: DeepReadonly<NextFontManifest>;
    isBot?: boolean;
    incrementalCache?: import('../lib/incremental-cache').IncrementalCache;
    isRevalidate?: boolean;
    nextExport?: boolean;
    nextConfigOutput?: 'standalone' | 'export';
    appDirDevErrorLogger?: (err: any) => Promise<void>;
    originalPathname?: string;
    isDraftMode?: boolean;
    deploymentId?: string;
    onUpdateCookies?: (cookies: string[]) => void;
    loadConfig?: (phase: string, dir: string, customConfig?: object | null, rawConfig?: boolean, silent?: boolean) => Promise<NextConfigComplete>;
    serverActions?: {
        bodySizeLimit?: SizeLimit;
        allowedOrigins?: string[];
    };
    params?: ParsedUrlQuery;
    isPrefetch?: boolean;
    experimental: {
        ppr: boolean;
        missingSuspenseWithCSRBailout: boolean;
        swrDelta: SwrDelta | undefined;
    };
    postponed?: string;
    /**
     * When true, only the skeleton of the PPR page will be rendered. This will
     * also enable other debugging features such as logging.
     */
    isDebugPPRSkeleton?: boolean;
    isStaticGeneration?: boolean;
}
export type RenderOpts = LoadComponentsReturnType<AppPageModule> & RenderOptsPartial;
export {};
