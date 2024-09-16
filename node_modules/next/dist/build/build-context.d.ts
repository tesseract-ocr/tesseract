import type { LoadedEnvFiles } from '@next/env';
import type { Rewrite, Redirect } from '../lib/load-custom-routes';
import type { __ApiPreviewProps } from '../server/api-utils';
import type { NextConfigComplete } from '../server/config-shared';
import type { Span } from '../trace';
import type getBaseWebpackConfig from './webpack-config';
import type { TelemetryPluginState } from './webpack/plugins/telemetry-plugin';
import type { Telemetry } from '../telemetry/storage';
export declare function resumePluginState(resumedState?: Record<string, any>): void;
export declare function getProxiedPluginState<State extends Record<string, any>>(initialState: State): State;
export declare function getPluginState(): Record<string, any>;
export interface MappedPages {
    [page: string]: string;
}
export declare const NextBuildContext: Partial<{
    compilerIdx?: number;
    pluginState: Record<string, any>;
    dir: string;
    buildId: string;
    encryptionKey: string;
    config: NextConfigComplete;
    appDir: string;
    pagesDir: string;
    rewrites: {
        fallback: Rewrite[];
        afterFiles: Rewrite[];
        beforeFiles: Rewrite[];
    };
    originalRewrites: {
        fallback: Rewrite[];
        afterFiles: Rewrite[];
        beforeFiles: Rewrite[];
    };
    originalRedirects: Redirect[];
    loadedEnvFiles: LoadedEnvFiles;
    previewProps: __ApiPreviewProps;
    mappedPages: MappedPages | undefined;
    mappedAppPages: MappedPages | undefined;
    mappedRootPaths: MappedPages;
    hasInstrumentationHook: boolean;
    telemetry: Telemetry;
    telemetryState: TelemetryPluginState;
    nextBuildSpan: Span;
    reactProductionProfiling: boolean;
    noMangling: boolean;
    appDirOnly: boolean;
    clientRouterFilters: Parameters<typeof getBaseWebpackConfig>[1]['clientRouterFilters'];
    previewModeId: string;
    fetchCacheKeyPrefix?: string;
    allowedRevalidateHeaderKeys?: string[];
}>;
