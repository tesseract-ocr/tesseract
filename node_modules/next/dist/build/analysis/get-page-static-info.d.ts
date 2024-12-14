import type { NextConfig } from '../../server/config-shared';
import type { RouteHas } from '../../lib/load-custom-routes';
import type { RSCMeta } from '../webpack/loaders/get-module-build-info';
import { PAGE_TYPES } from '../../lib/page-types';
import { type AppSegmentConfig } from '../segment-config/app/app-segment-config';
import { type PagesSegmentConfig, type PagesSegmentConfigConfig } from '../segment-config/pages/pages-segment-config';
export type MiddlewareMatcher = {
    regexp: string;
    locale?: false;
    has?: RouteHas[];
    missing?: RouteHas[];
    originalSource: string;
};
export type MiddlewareConfig = {
    /**
     * The matcher for the middleware. Read more: [Next.js Docs: Middleware `matcher`](https://nextjs.org/docs/app/api-reference/file-conventions/middleware#matcher),
     * [Next.js Docs: Middleware matching paths](https://nextjs.org/docs/app/building-your-application/routing/middleware#matching-paths)
     */
    matchers?: MiddlewareMatcher[];
    /**
     * The regions that the middleware should run in.
     */
    regions?: string | string[];
    /**
     * A glob, or an array of globs, ignoring dynamic code evaluation for specific
     * files. The globs are relative to your application root folder.
     */
    unstable_allowDynamic?: string[];
};
export interface AppPageStaticInfo {
    type: PAGE_TYPES.APP;
    ssg?: boolean;
    ssr?: boolean;
    rsc?: RSCModuleType;
    generateStaticParams?: boolean;
    generateSitemaps?: boolean;
    generateImageMetadata?: boolean;
    middleware?: MiddlewareConfig;
    config: Omit<AppSegmentConfig, 'runtime' | 'maxDuration'> | undefined;
    runtime: AppSegmentConfig['runtime'] | undefined;
    preferredRegion: AppSegmentConfig['preferredRegion'] | undefined;
    maxDuration: number | undefined;
}
export interface PagesPageStaticInfo {
    type: PAGE_TYPES.PAGES;
    getStaticProps?: boolean;
    getServerSideProps?: boolean;
    rsc?: RSCModuleType;
    generateStaticParams?: boolean;
    generateSitemaps?: boolean;
    generateImageMetadata?: boolean;
    middleware?: MiddlewareConfig;
    config: (Omit<PagesSegmentConfig, 'runtime' | 'config' | 'maxDuration'> & {
        config?: Omit<PagesSegmentConfigConfig, 'runtime' | 'maxDuration'>;
    }) | undefined;
    runtime: PagesSegmentConfig['runtime'] | undefined;
    preferredRegion: PagesSegmentConfigConfig['regions'] | undefined;
    maxDuration: number | undefined;
}
export type PageStaticInfo = AppPageStaticInfo | PagesPageStaticInfo;
export type RSCModuleType = 'server' | 'client';
export declare function getRSCModuleInformation(source: string, isReactServerLayer: boolean): RSCMeta;
export declare let hadUnsupportedValue: boolean;
type GetPageStaticInfoParams = {
    pageFilePath: string;
    nextConfig: Partial<NextConfig>;
    isDev?: boolean;
    page: string;
    pageType: PAGE_TYPES;
};
export declare function getAppPageStaticInfo({ pageFilePath, nextConfig, isDev, page, }: GetPageStaticInfoParams): Promise<AppPageStaticInfo>;
export declare function getPagesPageStaticInfo({ pageFilePath, nextConfig, isDev, page, }: GetPageStaticInfoParams): Promise<PagesPageStaticInfo>;
/**
 * For a given pageFilePath and nextConfig, if the config supports it, this
 * function will read the file and return the runtime that should be used.
 * It will look into the file content only if the page *requires* a runtime
 * to be specified, that is, when gSSP or gSP is used.
 * Related discussion: https://github.com/vercel/next.js/discussions/34179
 */
export declare function getPageStaticInfo(params: GetPageStaticInfoParams): Promise<PageStaticInfo>;
export {};
