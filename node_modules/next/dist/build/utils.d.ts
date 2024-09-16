import type { NextConfig, NextConfigComplete } from '../server/config-shared';
import type { AppBuildManifest } from './webpack/plugins/app-build-manifest-plugin';
import type { GetStaticPaths, GetStaticPathsResult, ServerRuntime } from 'next/types';
import type { BuildManifest } from '../server/get-page-files';
import type { CustomRoutes } from '../lib/load-custom-routes';
import type { MiddlewareManifest } from './webpack/plugins/middleware-plugin';
import type { WebpackLayerName } from '../lib/constants';
import type { AppPageModule } from '../server/future/route-modules/app-page/module';
import type { LoaderTree } from '../server/lib/app-dir-module';
import '../server/require-hook';
import '../server/node-polyfill-crypto';
import '../server/node-environment';
import { IncrementalCache } from '../server/lib/incremental-cache';
import type { PageExtensions } from './page-extensions-type';
export type ROUTER_TYPE = 'pages' | 'app';
export declare function unique<T>(main: ReadonlyArray<T>, sub: ReadonlyArray<T>): T[];
export declare function difference<T>(main: ReadonlyArray<T> | ReadonlySet<T>, sub: ReadonlyArray<T> | ReadonlySet<T>): T[];
type ComputeFilesGroup = {
    files: ReadonlyArray<string>;
    size: {
        total: number;
    };
};
type ComputeFilesManifest = {
    unique: ComputeFilesGroup;
    common: ComputeFilesGroup;
};
type ComputeFilesManifestResult = {
    router: {
        pages: ComputeFilesManifest;
        app?: ComputeFilesManifest;
    };
    sizes: Map<string, number>;
};
export declare function computeFromManifest(manifests: {
    build: BuildManifest;
    app?: AppBuildManifest;
}, distPath: string, gzipSize?: boolean, pageInfos?: Map<string, PageInfo>): Promise<ComputeFilesManifestResult>;
export declare function isMiddlewareFilename(file?: string): boolean;
export declare function isInstrumentationHookFilename(file?: string): boolean;
export interface PageInfo {
    isHybridAmp?: boolean;
    size: number;
    totalSize: number;
    isStatic: boolean;
    isSSG: boolean;
    isPPR: boolean;
    ssgPageRoutes: string[] | null;
    initialRevalidateSeconds: number | false;
    pageDuration: number | undefined;
    ssgPageDurations: number[] | undefined;
    runtime: ServerRuntime;
    hasEmptyPrelude?: boolean;
    hasPostponed?: boolean;
    isDynamicAppRoute?: boolean;
}
export type PageInfos = Map<string, PageInfo>;
export type SerializedPageInfos = [string, PageInfo][];
export declare function serializePageInfos(input: PageInfos): SerializedPageInfos;
export declare function deserializePageInfos(input: SerializedPageInfos): PageInfos;
export declare function printTreeView(lists: {
    pages: ReadonlyArray<string>;
    app: ReadonlyArray<string> | undefined;
}, pageInfos: Map<string, PageInfo>, { distPath, buildId, pagesDir, pageExtensions, buildManifest, appBuildManifest, middlewareManifest, useStaticPages404, gzipSize, }: {
    distPath: string;
    buildId: string;
    pagesDir?: string;
    pageExtensions: PageExtensions;
    buildManifest: BuildManifest;
    appBuildManifest?: AppBuildManifest;
    middlewareManifest: MiddlewareManifest;
    useStaticPages404: boolean;
    gzipSize?: boolean;
}): Promise<void>;
export declare function printCustomRoutes({ redirects, rewrites, headers, }: CustomRoutes): void;
export declare function getJsPageSizeInKb(routerType: ROUTER_TYPE, page: string, distPath: string, buildManifest: BuildManifest, appBuildManifest?: AppBuildManifest, gzipSize?: boolean, cachedStats?: ComputeFilesManifestResult): Promise<[number, number]>;
export declare function buildStaticPaths({ page, getStaticPaths, staticPathsResult, configFileName, locales, defaultLocale, appDir, }: {
    page: string;
    getStaticPaths?: GetStaticPaths;
    staticPathsResult?: GetStaticPathsResult;
    configFileName: string;
    locales?: string[];
    defaultLocale?: string;
    appDir?: boolean;
}): Promise<Omit<GetStaticPathsResult, 'paths'> & {
    paths: string[];
    encodedPaths: string[];
}>;
export type AppConfigDynamic = 'auto' | 'error' | 'force-static' | 'force-dynamic';
export type AppConfig = {
    revalidate?: number | false;
    dynamicParams?: true | false;
    dynamic?: AppConfigDynamic;
    fetchCache?: 'force-cache' | 'only-cache';
    preferredRegion?: string;
};
type Params = Record<string, string | string[]>;
type GenerateStaticParams = (options: {
    params?: Params;
}) => Promise<Params[]>;
type GenerateParamsResult = {
    config?: AppConfig;
    isDynamicSegment?: boolean;
    segmentPath: string;
    getStaticPaths?: GetStaticPaths;
    generateStaticParams?: GenerateStaticParams;
    isLayout?: boolean;
};
export type GenerateParamsResults = GenerateParamsResult[];
export declare const collectAppConfig: (mod: any) => AppConfig | undefined;
/**
 * Walks the loader tree and collects the generate parameters for each segment.
 *
 * @param tree the loader tree
 * @returns the generate parameters for each segment
 */
export declare function collectGenerateParams(tree: LoaderTree): Promise<GenerateParamsResults>;
export declare function buildAppStaticPaths({ dir, page, distDir, configFileName, generateParams, isrFlushToDisk, cacheHandler, requestHeaders, maxMemoryCacheSize, fetchCacheKeyPrefix, ppr, ComponentMod, }: {
    dir: string;
    page: string;
    configFileName: string;
    generateParams: GenerateParamsResults;
    distDir: string;
    isrFlushToDisk?: boolean;
    fetchCacheKeyPrefix?: string;
    cacheHandler?: string;
    maxMemoryCacheSize?: number;
    requestHeaders: IncrementalCache['requestHeaders'];
    ppr: boolean;
    ComponentMod: AppPageModule;
}): Promise<(Omit<GetStaticPathsResult, "paths"> & {
    paths: string[];
    encodedPaths: string[];
}) | {
    paths: undefined;
    fallback: boolean | undefined;
    encodedPaths: undefined;
}>;
export declare function isPageStatic({ dir, page, distDir, configFileName, runtimeEnvConfig, httpAgentOptions, locales, defaultLocale, parentId, pageRuntime, edgeInfo, pageType, originalAppPath, isrFlushToDisk, maxMemoryCacheSize, cacheHandler, ppr, }: {
    dir: string;
    page: string;
    distDir: string;
    configFileName: string;
    runtimeEnvConfig: any;
    httpAgentOptions: NextConfigComplete['httpAgentOptions'];
    locales?: string[];
    defaultLocale?: string;
    parentId?: any;
    edgeInfo?: any;
    pageType?: 'pages' | 'app';
    pageRuntime?: ServerRuntime;
    originalAppPath?: string;
    isrFlushToDisk?: boolean;
    maxMemoryCacheSize?: number;
    cacheHandler?: string;
    nextConfigOutput: 'standalone' | 'export';
    ppr: boolean;
}): Promise<{
    isPPR?: boolean;
    isStatic?: boolean;
    isAmpOnly?: boolean;
    isHybridAmp?: boolean;
    hasServerProps?: boolean;
    hasStaticProps?: boolean;
    prerenderRoutes?: string[];
    encodedPrerenderRoutes?: string[];
    prerenderFallback?: boolean | 'blocking';
    isNextImageImported?: boolean;
    traceIncludes?: string[];
    traceExcludes?: string[];
    appConfig?: AppConfig;
}>;
export declare function hasCustomGetInitialProps({ page, distDir, runtimeEnvConfig, checkingApp, }: {
    page: string;
    distDir: string;
    runtimeEnvConfig: any;
    checkingApp: boolean;
}): Promise<boolean>;
export declare function getDefinedNamedExports({ page, distDir, runtimeEnvConfig, }: {
    page: string;
    distDir: string;
    runtimeEnvConfig: any;
}): Promise<ReadonlyArray<string>>;
export declare function detectConflictingPaths(combinedPages: string[], ssgPages: Set<string>, additionalSsgPaths: Map<string, string[]>): void;
export declare function copyTracedFiles(dir: string, distDir: string, pageKeys: readonly string[], appPageKeys: readonly string[] | undefined, tracingRoot: string, serverConfig: NextConfig, middlewareManifest: MiddlewareManifest, hasInstrumentationHook: boolean, staticPages: Set<string>): Promise<void>;
export declare function isReservedPage(page: string): boolean;
export declare function isAppBuiltinNotFoundPage(page: string): boolean;
export declare function isCustomErrorPage(page: string): boolean;
export declare function isMiddlewareFile(file: string): boolean;
export declare function isInstrumentationHookFile(file: string): boolean;
export declare function getPossibleInstrumentationHookFilenames(folder: string, extensions: string[]): string[];
export declare function getPossibleMiddlewareFilenames(folder: string, extensions: string[]): string[];
export declare class NestedMiddlewareError extends Error {
    constructor(nestedFileNames: string[], mainDir: string, pagesOrAppDir: string);
}
export declare function getSupportedBrowsers(dir: string, isDevelopment: boolean): string[] | undefined;
export declare function isWebpackServerOnlyLayer(layer: WebpackLayerName | null | undefined): boolean;
export declare function isWebpackClientOnlyLayer(layer: WebpackLayerName | null | undefined): boolean;
export declare function isWebpackDefaultLayer(layer: WebpackLayerName | null | undefined): boolean;
export declare function isWebpackAppLayer(layer: WebpackLayerName | null | undefined): boolean;
export {};
