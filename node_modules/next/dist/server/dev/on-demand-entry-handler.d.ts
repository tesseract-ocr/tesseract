import type ws from 'next/dist/compiled/ws';
import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { NextConfigComplete } from '../config-shared';
import type { CompilerNameValues } from '../../shared/lib/constants';
import type { RouteDefinition } from '../route-definitions/route-definition';
import type HotReloaderWebpack from './hot-reloader-webpack';
import { COMPILER_INDEXES } from '../../shared/lib/constants';
import { PAGE_TYPES } from '../../lib/page-types';
declare const COMPILER_KEYS: CompilerNameValues[];
/**
 * format: {compiler type}@{page type}@{page path}
 * e.g. client@pages@/index
 * e.g. server@app@app/page
 *
 * This guarantees the uniqueness for each page, to avoid conflicts between app/ and pages/
 */
export declare function getEntryKey(compilerType: CompilerNameValues, pageBundleType: PAGE_TYPES, page: string): string;
export declare const ADDED: unique symbol;
export declare const BUILDING: unique symbol;
export declare const BUILT: unique symbol;
interface EntryType {
    /**
     * Tells if a page is scheduled to be disposed.
     */
    dispose?: boolean;
    /**
     * Timestamp with the last time the page was active.
     */
    lastActiveTime?: number;
    /**
     * Page build status.
     */
    status?: typeof ADDED | typeof BUILDING | typeof BUILT;
    /**
     * Path to the page file relative to the dist folder with no extension.
     * For example: `pages/about/index`
     */
    bundlePath: string;
    /**
     * Webpack request to create a dependency for.
     */
    request: string;
}
export declare const enum EntryTypes {
    ENTRY = 0,
    CHILD_ENTRY = 1
}
interface Entry extends EntryType {
    type: EntryTypes.ENTRY;
    /**
     * The absolute page to the page file. Used for detecting if the file was removed. For example:
     * `/Users/Rick/project/pages/about/index.js`
     */
    absolutePagePath: string;
    /**
     * All parallel pages that match the same entry, for example:
     * ['/parallel/@bar/nested/@a/page', '/parallel/@bar/nested/@b/page', '/parallel/@foo/nested/@a/page', '/parallel/@foo/nested/@b/page']
     */
    appPaths: ReadonlyArray<string> | null;
}
interface ChildEntry extends EntryType {
    type: EntryTypes.CHILD_ENTRY;
    /**
     * Which parent entries use this childEntry.
     */
    parentEntries: Set<string>;
    /**
     * The absolute page to the entry file. Used for detecting if the file was removed. For example:
     * `/Users/Rick/project/app/about/layout.js`
     */
    absoluteEntryFilePath?: string;
}
declare const entriesMap: Map<string, {
    /**
     * The key composed of the compiler name and the page. For example:
     * `edge-server/about`
     */
    [entryName: string]: Entry | ChildEntry;
}>;
export declare const getEntries: (dir: string) => NonNullable<ReturnType<(typeof entriesMap)["get"]>>;
export declare const getInvalidator: (dir: string) => Invalidator | undefined;
declare class Invalidator {
    private multiCompiler;
    private building;
    private rebuildAgain;
    constructor(multiCompiler: webpack.MultiCompiler);
    shouldRebuildAll(): boolean;
    invalidate(compilerKeys?: typeof COMPILER_KEYS): void;
    startBuilding(compilerKey: keyof typeof COMPILER_INDEXES): void;
    doneBuilding(compilerKeys?: typeof COMPILER_KEYS): void;
    willRebuild(compilerKey: keyof typeof COMPILER_INDEXES): boolean;
}
interface PagePathData {
    filename: string;
    bundlePath: string;
    page: string;
}
/**
 * Attempts to find a page file path from the given pages absolute directory,
 * a page and allowed extensions. If the page can't be found it will throw an
 * error. It defaults the `/_error` page to Next.js internal error page.
 *
 * @param rootDir Absolute path to the project root.
 * @param page The page normalized (it will be denormalized).
 * @param extensions Array of page extensions.
 * @param pagesDir Absolute path to the pages folder with trailing `/pages`.
 * @param appDir Absolute path to the app folder with trailing `/app`.
 */
export declare function findPagePathData(rootDir: string, page: string, extensions: string[], pagesDir?: string, appDir?: string): Promise<PagePathData>;
export declare function onDemandEntryHandler({ hotReloader, maxInactiveAge, multiCompiler, nextConfig, pagesBufferLength, pagesDir, rootDir, appDir, }: {
    hotReloader: HotReloaderWebpack;
    maxInactiveAge: number;
    multiCompiler: webpack.MultiCompiler;
    nextConfig: NextConfigComplete;
    pagesBufferLength: number;
    pagesDir?: string;
    rootDir: string;
    appDir?: string;
}): {
    ensurePage({ page, appPaths, definition, isApp, url, }: {
        page: string;
        appPaths?: ReadonlyArray<string> | null;
        definition?: RouteDefinition;
        isApp?: boolean;
        url?: string;
    }): Promise<void>;
    onHMR(client: ws, getHmrServerError: () => Error | null): void;
};
export {};
