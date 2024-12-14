import type { NextConfigComplete } from '../server/config-shared';
import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { MiddlewareConfig, PageStaticInfo } from './analysis/get-page-static-info';
import type { LoadedEnvFiles } from '@next/env';
import type { AppLoaderOptions } from './webpack/loaders/next-app-loader';
import type { CompilerNameValues } from '../shared/lib/constants';
import type { __ApiPreviewProps } from '../server/api-utils';
import type { ServerRuntime } from '../types';
import type { PageExtensions } from './page-extensions-type';
import type { MappedPages } from './build-context';
import { PAGE_TYPES } from '../lib/page-types';
export declare function sortByPageExts(pageExtensions: PageExtensions): (a: string, b: string) => number;
export declare function getStaticInfoIncludingLayouts({ isInsideAppDir, pageExtensions, pageFilePath, appDir, config: nextConfig, isDev, page, }: {
    isInsideAppDir: boolean;
    pageExtensions: PageExtensions;
    pageFilePath: string;
    appDir: string | undefined;
    config: NextConfigComplete;
    isDev: boolean | undefined;
    page: string;
}): Promise<PageStaticInfo>;
type ObjectValue<T> = T extends {
    [key: string]: infer V;
} ? V : never;
/**
 * For a given page path removes the provided extensions.
 */
export declare function getPageFromPath(pagePath: string, pageExtensions: PageExtensions): string;
export declare function getPageFilePath({ absolutePagePath, pagesDir, appDir, rootDir, }: {
    absolutePagePath: string;
    pagesDir: string | undefined;
    appDir: string | undefined;
    rootDir: string;
}): string;
/**
 * Creates a mapping of route to page file path for a given list of page paths.
 * For example ['/middleware.ts'] is turned into  { '/middleware': `${ROOT_DIR_ALIAS}/middleware.ts` }
 */
export declare function createPagesMapping({ isDev, pageExtensions, pagePaths, pagesType, pagesDir, appDir, }: {
    isDev: boolean;
    pageExtensions: PageExtensions;
    pagePaths: string[];
    pagesType: PAGE_TYPES;
    pagesDir: string | undefined;
    appDir: string | undefined;
}): Promise<MappedPages>;
export interface CreateEntrypointsParams {
    buildId: string;
    config: NextConfigComplete;
    envFiles: LoadedEnvFiles;
    isDev?: boolean;
    pages: MappedPages;
    pagesDir?: string;
    previewMode: __ApiPreviewProps;
    rootDir: string;
    rootPaths?: MappedPages;
    appDir?: string;
    appPaths?: MappedPages;
    pageExtensions: PageExtensions;
    hasInstrumentationHook?: boolean;
}
export declare function getEdgeServerEntry(opts: {
    rootDir: string;
    absolutePagePath: string;
    buildId: string;
    bundlePath: string;
    config: NextConfigComplete;
    isDev: boolean;
    isServerComponent: boolean;
    page: string;
    pages: MappedPages;
    middleware?: Partial<MiddlewareConfig>;
    pagesType: PAGE_TYPES;
    appDirLoader?: string;
    hasInstrumentationHook?: boolean;
    preferredRegion: string | string[] | undefined;
    middlewareConfig?: MiddlewareConfig;
}): string | {
    import: string;
    layer: "rsc";
} | {
    import: string;
    layer: "ssr" | undefined;
};
export declare function getInstrumentationEntry(opts: {
    absolutePagePath: string;
    isEdgeServer: boolean;
    isDev: boolean;
}): {
    import: string;
    filename: string;
    layer: "instrument";
};
export declare function getAppEntry(opts: Readonly<AppLoaderOptions>): {
    import: string;
    layer: "rsc";
};
export declare function getClientEntry(opts: {
    absolutePagePath: string;
    page: string;
}): string | string[];
export declare function runDependingOnPageType<T>(params: {
    onClient: () => T;
    onEdgeServer: () => T;
    onServer: () => T;
    page: string;
    pageRuntime: ServerRuntime;
    pageType?: PAGE_TYPES;
}): void;
export declare function createEntrypoints(params: CreateEntrypointsParams): Promise<{
    client: webpack.EntryObject;
    server: webpack.EntryObject;
    edgeServer: webpack.EntryObject;
    middlewareMatchers: undefined;
}>;
export declare function finalizeEntrypoint({ name, compilerType, value, isServerComponent, hasAppDir, }: {
    compilerType?: CompilerNameValues;
    name: string;
    value: ObjectValue<webpack.EntryObject>;
    isServerComponent?: boolean;
    hasAppDir?: boolean;
}): ObjectValue<webpack.EntryObject>;
export {};
