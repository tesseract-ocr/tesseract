import type { ManifestRoute, PrerenderManifest } from '../../../build';
import type { NextConfigComplete } from '../../config-shared';
import type { PatchMatcher } from '../../../shared/lib/router/utils/path-match';
import type { MiddlewareRouteMatch } from '../../../shared/lib/router/utils/middleware-route-matcher';
import { type Rewrite } from '../../../lib/load-custom-routes';
export type FsOutput = {
    type: 'appFile' | 'pageFile' | 'nextImage' | 'publicFolder' | 'nextStaticFolder' | 'legacyStaticFolder' | 'devVirtualFsItem';
    itemPath: string;
    fsPath?: string;
    itemsRoot?: string;
    locale?: string;
};
export type FilesystemDynamicRoute = ManifestRoute & {
    /**
     * The path matcher that can be used to match paths against this route.
     */
    match: PatchMatcher;
};
export declare const buildCustomRoute: <T>(type: "redirect" | "header" | "rewrite" | "before_files_rewrite", item: T & {
    source: string;
}, basePath?: string, caseSensitive?: boolean) => T & {
    match: PatchMatcher;
    check?: boolean;
};
export declare function setupFsCheck(opts: {
    dir: string;
    dev: boolean;
    minimalMode?: boolean;
    config: NextConfigComplete;
    addDevWatcherCallback?: (arg: (files: Map<string, {
        timestamp: number;
    }>) => void) => void;
}): Promise<{
    headers: (import("../../../lib/load-custom-routes").Header & {
        match: PatchMatcher;
        check?: boolean;
    })[];
    rewrites: {
        beforeFiles: (Rewrite & {
            match: PatchMatcher;
            check?: boolean;
        })[];
        afterFiles: (Rewrite & {
            match: PatchMatcher;
            check?: boolean;
        })[];
        fallback: (Rewrite & {
            match: PatchMatcher;
            check?: boolean;
        })[];
    };
    redirects: (import("../../../lib/load-custom-routes").Redirect & {
        match: PatchMatcher;
        check?: boolean;
    })[];
    buildId: string;
    handleLocale: (pathname: string, locales?: string[]) => {
        locale: string | undefined;
        pathname: string;
    };
    appFiles: Set<string>;
    pageFiles: Set<string>;
    dynamicRoutes: FilesystemDynamicRoute[];
    nextDataRoutes: Set<string>;
    exportPathMapRoutes: undefined | ReturnType<typeof buildCustomRoute<Rewrite>>[];
    devVirtualFsItems: Set<string>;
    prerenderManifest: PrerenderManifest;
    middlewareMatcher: MiddlewareRouteMatch | undefined;
    ensureCallback(fn: (item: FsOutput) => Promise<void> | undefined): void;
    getItem(itemPath: string): Promise<FsOutput | null>;
    getDynamicRoutes(): FilesystemDynamicRoute[];
    getMiddlewareMatchers(): MiddlewareRouteMatch | undefined;
}>;
