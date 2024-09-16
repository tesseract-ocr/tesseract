import type { ComponentType } from 'react';
import type { MiddlewareMatcher } from '../build/analysis/get-page-static-info';
declare global {
    interface Window {
        __BUILD_MANIFEST?: Record<string, string[]>;
        __BUILD_MANIFEST_CB?: Function;
        __MIDDLEWARE_MATCHERS?: MiddlewareMatcher[];
        __MIDDLEWARE_MANIFEST_CB?: Function;
        __REACT_LOADABLE_MANIFEST?: any;
        __RSC_MANIFEST?: any;
        __RSC_SERVER_MANIFEST?: any;
        __NEXT_FONT_MANIFEST?: any;
        __SUBRESOURCE_INTEGRITY_MANIFEST?: string;
        __INTERCEPTION_ROUTE_REWRITE_MANIFEST?: string;
    }
}
interface LoadedEntrypointSuccess {
    component: ComponentType;
    exports: any;
}
interface LoadedEntrypointFailure {
    error: unknown;
}
type RouteEntrypoint = LoadedEntrypointSuccess | LoadedEntrypointFailure;
interface RouteStyleSheet {
    href: string;
    content: string;
}
interface LoadedRouteSuccess extends LoadedEntrypointSuccess {
    styles: RouteStyleSheet[];
}
interface LoadedRouteFailure {
    error: unknown;
}
type RouteLoaderEntry = LoadedRouteSuccess | LoadedRouteFailure;
export interface RouteLoader {
    whenEntrypoint(route: string): Promise<RouteEntrypoint>;
    onEntrypoint(route: string, execute: () => unknown): void;
    loadRoute(route: string, prefetch?: boolean): Promise<RouteLoaderEntry>;
    prefetch(route: string): Promise<void>;
}
export declare function markAssetError(err: Error): Error;
export declare function isAssetError(err?: Error): boolean | undefined;
export declare function getClientBuildManifest(): Promise<Record<string, string[]>>;
export declare function createRouteLoader(assetPrefix: string): RouteLoader;
export {};
