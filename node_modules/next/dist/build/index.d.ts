import type { NextConfigComplete } from '../server/config-shared';
import type { Revalidate } from '../server/lib/revalidate';
import '../lib/setup-exception-listeners';
import { Worker } from '../lib/worker';
import { RSC_PREFETCH_SUFFIX, RSC_SUFFIX } from '../lib/constants';
import type { Header, Redirect, Rewrite, RouteHas } from '../lib/load-custom-routes';
import type { __ApiPreviewProps } from '../server/api-utils';
import { NEXT_ROUTER_PREFETCH_HEADER, RSC_HEADER, RSC_CONTENT_TYPE_HEADER, NEXT_DID_POSTPONE_HEADER } from '../client/components/app-router-headers';
import { RenderingMode } from './rendering-mode';
type Fallback = null | boolean | string;
export interface SsgRoute {
    dataRoute: string | null;
    experimentalBypassFor?: RouteHas[];
    /**
     * The headers that should be served along side this prerendered route.
     */
    initialHeaders?: Record<string, string>;
    /**
     * The status code that should be served along side this prerendered route.
     */
    initialStatus?: number;
    /**
     * The revalidation configuration for this route.
     */
    initialRevalidateSeconds: Revalidate;
    /**
     * The prefetch data route associated with this page. If not defined, this
     * page does not support prefetching.
     */
    prefetchDataRoute: string | null | undefined;
    /**
     * The dynamic route that this statically prerendered route is based on. If
     * this is null, then the route was not based on a dynamic route.
     */
    srcRoute: string | null;
    /**
     * @deprecated use `renderingMode` instead
     */
    experimentalPPR: boolean | undefined;
    /**
     * The rendering mode for this route. Only `undefined` when not an app router
     * route.
     */
    renderingMode: RenderingMode | undefined;
    /**
     * The headers that are allowed to be used when revalidating this route. These
     * are used internally by Next.js to revalidate routes.
     */
    allowHeader: string[];
}
export interface DynamicSsgRoute {
    dataRoute: string | null;
    dataRouteRegex: string | null;
    experimentalBypassFor?: RouteHas[];
    fallback: Fallback;
    /**
     * When defined, it describes the revalidation configuration for the fallback
     * route.
     */
    fallbackRevalidate: Revalidate | undefined;
    /**
     * The headers that should used when serving the fallback.
     */
    fallbackHeaders?: Record<string, string>;
    /**
     * The status code that should be used when serving the fallback.
     */
    fallbackStatus?: number;
    prefetchDataRoute: string | null | undefined;
    prefetchDataRouteRegex: string | null | undefined;
    routeRegex: string;
    /**
     * @deprecated use `renderingMode` instead
     */
    experimentalPPR: boolean | undefined;
    /**
     * The rendering mode for this route. Only `undefined` when not an app router
     * route.
     */
    renderingMode: RenderingMode | undefined;
    /**
     * The headers that are allowed to be used when revalidating this route. These
     * are used internally by Next.js to revalidate routes.
     */
    allowHeader: string[];
}
export type PrerenderManifest = {
    version: 4;
    routes: {
        [route: string]: SsgRoute;
    };
    dynamicRoutes: {
        [route: string]: DynamicSsgRoute;
    };
    notFoundRoutes: string[];
    preview: __ApiPreviewProps;
};
type ManifestBuiltRoute = {
    /**
     * The route pattern used to match requests for this route.
     */
    regex: string;
};
export type ManifestRewriteRoute = ManifestBuiltRoute & Rewrite;
export type ManifestRedirectRoute = ManifestBuiltRoute & Redirect;
export type ManifestHeaderRoute = ManifestBuiltRoute & Header;
export type ManifestRoute = ManifestBuiltRoute & {
    page: string;
    namedRegex?: string;
    routeKeys?: {
        [key: string]: string;
    };
};
export type ManifestDataRoute = {
    page: string;
    routeKeys?: {
        [key: string]: string;
    };
    dataRouteRegex: string;
    namedDataRouteRegex?: string;
};
export type RoutesManifest = {
    version: number;
    pages404: boolean;
    basePath: string;
    redirects: Array<Redirect>;
    rewrites?: Array<ManifestRewriteRoute> | {
        beforeFiles: Array<ManifestRewriteRoute>;
        afterFiles: Array<ManifestRewriteRoute>;
        fallback: Array<ManifestRewriteRoute>;
    };
    headers: Array<ManifestHeaderRoute>;
    staticRoutes: Array<ManifestRoute>;
    dynamicRoutes: Array<ManifestRoute>;
    dataRoutes: Array<ManifestDataRoute>;
    i18n?: {
        domains?: Array<{
            http?: true;
            domain: string;
            locales?: string[];
            defaultLocale: string;
        }>;
        locales: string[];
        defaultLocale: string;
        localeDetection?: false;
    };
    rsc: {
        header: typeof RSC_HEADER;
        didPostponeHeader: typeof NEXT_DID_POSTPONE_HEADER;
        contentTypeHeader: typeof RSC_CONTENT_TYPE_HEADER;
        varyHeader: string;
        prefetchHeader: typeof NEXT_ROUTER_PREFETCH_HEADER;
        suffix: typeof RSC_SUFFIX;
        prefetchSuffix: typeof RSC_PREFETCH_SUFFIX;
    };
    skipMiddlewareUrlNormalize?: boolean;
    caseSensitive?: boolean;
    /**
     * Configuration related to Partial Prerendering.
     */
    ppr?: {
        /**
         * The chained response for the PPR resume.
         */
        chain: {
            /**
             * The headers that will indicate to Next.js that the request is for a PPR
             * resume.
             */
            headers: Record<string, string>;
        };
    };
};
type StaticWorker = typeof import('./worker') & Worker;
export declare function createStaticWorker(config: NextConfigComplete, progress?: {
    run: () => void;
    clear: () => void;
}): StaticWorker;
export default function build(dir: string, reactProductionProfiling: boolean | undefined, debugOutput: boolean | undefined, runLint: boolean | undefined, noMangling: boolean | undefined, appDirOnly: boolean | undefined, turboNextBuild: boolean | undefined, experimentalBuildMode: 'default' | 'compile' | 'generate', traceUploadUrl: string | undefined): Promise<void>;
export {};
