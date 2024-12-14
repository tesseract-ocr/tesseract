import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { MiddlewareConfig } from '../../../analysis/get-page-static-info';
import { RouteKind } from '../../../../server/route-kind';
import type { MappedPages } from '../../../build-context';
type RouteLoaderOptionsPagesAPIInput = {
    kind: RouteKind.PAGES_API;
    page: string;
    preferredRegion: string | string[] | undefined;
    absolutePagePath: string;
    middlewareConfig: MiddlewareConfig;
};
type RouteLoaderOptionsPagesInput = {
    kind: RouteKind.PAGES;
    page: string;
    pages: MappedPages;
    preferredRegion: string | string[] | undefined;
    absolutePagePath: string;
    middlewareConfig: MiddlewareConfig;
};
type RouteLoaderOptionsInput = RouteLoaderOptionsPagesInput | RouteLoaderOptionsPagesAPIInput;
type RouteLoaderPagesAPIOptions = {
    kind: RouteKind.PAGES_API;
    /**
     * The page name for this particular route.
     */
    page: string;
    /**
     * The preferred region for this route.
     */
    preferredRegion: string | string[] | undefined;
    /**
     * The absolute path to the userland page file.
     */
    absolutePagePath: string;
    /**
     * The middleware config for this route.
     */
    middlewareConfigBase64: string;
};
type RouteLoaderPagesOptions = {
    kind: RouteKind.PAGES;
    /**
     * The page name for this particular route.
     */
    page: string;
    /**
     * The preferred region for this route.
     */
    preferredRegion: string | string[] | undefined;
    /**
     * The absolute path to the userland page file.
     */
    absolutePagePath: string;
    /**
     * The absolute paths to the app path file.
     */
    absoluteAppPath: string;
    /**
     * The absolute paths to the document path file.
     */
    absoluteDocumentPath: string;
    /**
     * The middleware config for this route.
     */
    middlewareConfigBase64: string;
};
/**
 * The options for the route loader.
 */
type RouteLoaderOptions = RouteLoaderPagesOptions | RouteLoaderPagesAPIOptions;
/**
 * Returns the loader entry for a given page.
 *
 * @param options the options to create the loader entry
 * @returns the encoded loader entry
 */
export declare function getRouteLoaderEntry(options: RouteLoaderOptionsInput): string;
/**
 * Handles the `next-route-loader` options.
 * @returns the loader definition function
 */
declare const loader: webpack.LoaderDefinitionFunction<RouteLoaderOptions>;
export default loader;
