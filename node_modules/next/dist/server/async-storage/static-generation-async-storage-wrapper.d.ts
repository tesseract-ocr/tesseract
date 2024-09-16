import type { AsyncStorageWrapper } from './async-storage-wrapper';
import type { StaticGenerationStore } from '../../client/components/static-generation-async-storage.external';
import type { IncrementalCache } from '../lib/incremental-cache';
import type { RenderOptsPartial } from '../app-render/types';
import type { FetchMetric } from '../base-http';
export type StaticGenerationContext = {
    urlPathname: string;
    requestEndedState?: {
        ended?: boolean;
    };
    renderOpts: {
        incrementalCache?: IncrementalCache;
        isOnDemandRevalidate?: boolean;
        fetchCache?: StaticGenerationStore['fetchCache'];
        isServerAction?: boolean;
        waitUntil?: Promise<any>;
        experimental: {
            ppr: boolean;
            missingSuspenseWithCSRBailout?: boolean;
        };
        /**
         * Fetch metrics attached in patch-fetch.ts
         **/
        fetchMetrics?: FetchMetric[];
    } & Pick<RenderOptsPartial, 'originalPathname' | 'supportsDynamicHTML' | 'isRevalidate' | 'nextExport' | 'isDraftMode' | 'isDebugPPRSkeleton'>;
};
export declare const StaticGenerationAsyncStorageWrapper: AsyncStorageWrapper<StaticGenerationStore, StaticGenerationContext>;
