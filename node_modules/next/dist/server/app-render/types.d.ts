import type { LoadComponentsReturnType } from '../load-components';
import type { ServerRuntime, SizeLimit } from '../../types';
import type { NextConfigComplete } from '../../server/config-shared';
import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { NextFontManifest } from '../../build/webpack/plugins/next-font-manifest-plugin';
import type { ParsedUrlQuery } from 'querystring';
import type { AppPageModule } from '../route-modules/app-page/module';
import type { ExpireTime } from '../lib/revalidate';
import type { LoadingModuleData } from '../../shared/lib/app-router-context.shared-runtime';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
import type { __ApiPreviewProps } from '../api-utils';
import s from 'next/dist/compiled/superstruct';
import type { RequestLifecycleOpts } from '../base-server';
import type { InstrumentationOnRequestError } from '../instrumentation/types';
import type { NextRequestHint } from '../web/adapter';
import type { BaseNextRequest } from '../base-http';
import type { IncomingMessage } from 'http';
import type { RenderResumeDataCache } from '../resume-data-cache/resume-data-cache';
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
    node: React.ReactNode | null,
    parallelRoutes: {
        [parallelRouterKey: string]: CacheNodeSeedData | null;
    },
    loading: LoadingModuleData | Promise<LoadingModuleData>,
    isPartial: boolean
];
export type FlightDataSegment = [
    Segment,
    FlightRouterState,
    CacheNodeSeedData | null,
    // Can be null during prefetch if there's no loading component
    React.ReactNode | null,
    boolean
];
export type FlightDataPath = any[] | [
    ...FlightSegmentPath[],
    ...FlightDataSegment
];
/**
 * The Flight response data
 */
export type FlightData = Array<FlightDataPath> | string;
export type ActionResult = Promise<any>;
export type ServerOnInstrumentationRequestError = (error: unknown, request: NextRequestHint | BaseNextRequest | IncomingMessage, errorContext: Parameters<InstrumentationOnRequestError>[2]) => void | Promise<void>;
export interface RenderOptsPartial {
    previewProps: __ApiPreviewProps | undefined;
    err?: Error | null;
    dev?: boolean;
    buildId: string;
    basePath: string;
    trailingSlash: boolean;
    clientReferenceManifest?: DeepReadonly<ClientReferenceManifest>;
    supportsDynamicResponse: boolean;
    runtime?: ServerRuntime;
    serverComponents?: boolean;
    enableTainting?: boolean;
    assetPrefix?: string;
    crossOrigin?: '' | 'anonymous' | 'use-credentials' | undefined;
    nextFontManifest?: DeepReadonly<NextFontManifest>;
    isBot?: boolean;
    incrementalCache?: import('../lib/incremental-cache').IncrementalCache;
    cacheLifeProfiles?: {
        [profile: string]: import('../use-cache/cache-life').CacheLife;
    };
    setAppIsrStatus?: (key: string, value: boolean | null) => void;
    isRevalidate?: boolean;
    nextExport?: boolean;
    nextConfigOutput?: 'standalone' | 'export';
    onInstrumentationRequestError?: ServerOnInstrumentationRequestError;
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
        /**
         * When true, it indicates that the current page supports partial
         * prerendering.
         */
        isRoutePPREnabled?: boolean;
        expireTime: ExpireTime | undefined;
        clientTraceMetadata: string[] | undefined;
        dynamicIO: boolean;
        inlineCss: boolean;
        authInterrupts: boolean;
    };
    postponed?: string;
    /**
     * The resume data cache that was generated for this partially prerendered
     * page during dev warmup.
     */
    devRenderResumeDataCache?: RenderResumeDataCache;
    /**
     * When true, only the static shell of the page will be rendered. This will
     * also enable other debugging features such as logging in development.
     */
    isDebugStaticShell?: boolean;
    /**
     * When true, the page will be rendered using the static rendering to detect
     * any dynamic API's that would have stopped the page from being fully
     * statically generated.
     */
    isDebugDynamicAccesses?: boolean;
    /**
     * The maximum length of the headers that are emitted by React and added to
     * the response.
     */
    reactMaxHeadersLength: number | undefined;
    isStaticGeneration?: boolean;
}
export type RenderOpts = LoadComponentsReturnType<AppPageModule> & RenderOptsPartial & RequestLifecycleOpts;
export type PreloadCallbacks = (() => void)[];
export type InitialRSCPayload = {
    /** buildId */
    b: string;
    /** assetPrefix */
    p: string;
    /** initialCanonicalUrlParts */
    c: string[];
    /** couldBeIntercepted */
    i: boolean;
    /** initialFlightData */
    f: FlightDataPath[];
    /** missingSlots */
    m: Set<string> | undefined;
    /** GlobalError */
    G: [React.ComponentType<any>, React.ReactNode | undefined];
    /** postponed */
    s: boolean;
    /** prerendered */
    S: boolean;
};
export type NavigationFlightResponse = {
    /** buildId */
    b: string;
    /** flightData */
    f: FlightData;
    /** prerendered */
    S: boolean;
};
export type ActionFlightResponse = {
    /** actionResult */
    a: ActionResult;
    /** buildId */
    b: string;
    /** flightData */
    f: FlightData;
};
export type RSCPayload = InitialRSCPayload | NavigationFlightResponse | ActionFlightResponse;
export {};
