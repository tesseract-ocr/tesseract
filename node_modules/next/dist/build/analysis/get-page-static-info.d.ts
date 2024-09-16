import type { NextConfig } from '../../server/config-shared';
import type { RouteHas } from '../../lib/load-custom-routes';
import type { ServerRuntime } from 'next/types';
import type { RSCMeta } from '../webpack/loaders/get-module-build-info';
import { PAGE_TYPES } from '../../lib/page-types';
export interface MiddlewareConfigParsed extends Omit<MiddlewareConfig, 'matcher'> {
    matchers?: MiddlewareMatcher[];
}
/**
 * This interface represents the exported `config` object in a `middleware.ts` file.
 *
 * Read more: [Next.js Docs: Middleware `config` object](https://nextjs.org/docs/app/api-reference/file-conventions/middleware#config-object-optional)
 */
export interface MiddlewareConfig {
    /**
     * Read more: [Next.js Docs: Middleware `matcher`](https://nextjs.org/docs/app/api-reference/file-conventions/middleware#matcher),
     * [Next.js Docs: Middleware matching paths](https://nextjs.org/docs/app/building-your-application/routing/middleware#matching-paths)
     */
    matcher?: string | string[] | MiddlewareMatcher[];
    unstable_allowDynamicGlobs?: string[];
    regions?: string[] | string;
}
export interface MiddlewareMatcher {
    regexp: string;
    locale?: false;
    has?: RouteHas[];
    missing?: RouteHas[];
    originalSource: string;
}
export interface PageStaticInfo {
    runtime?: ServerRuntime;
    preferredRegion?: string | string[];
    ssg?: boolean;
    ssr?: boolean;
    rsc?: RSCModuleType;
    generateStaticParams?: boolean;
    middleware?: MiddlewareConfigParsed;
    amp?: boolean | 'hybrid';
    extraConfig?: Record<string, any>;
}
export type RSCModuleType = 'server' | 'client';
export declare function getRSCModuleInformation(source: string, isReactServerLayer: boolean): RSCMeta;
export declare function getMiddlewareMatchers(matcherOrMatchers: unknown, nextConfig: NextConfig): MiddlewareMatcher[];
export declare function isDynamicMetadataRoute(pageFilePath: string): Promise<boolean>;
/**
 * For a given pageFilePath and nextConfig, if the config supports it, this
 * function will read the file and return the runtime that should be used.
 * It will look into the file content only if the page *requires* a runtime
 * to be specified, that is, when gSSP or gSP is used.
 * Related discussion: https://github.com/vercel/next.js/discussions/34179
 */
export declare function getPageStaticInfo(params: {
    pageFilePath: string;
    nextConfig: Partial<NextConfig>;
    isDev?: boolean;
    page?: string;
    pageType: PAGE_TYPES;
}): Promise<PageStaticInfo>;
