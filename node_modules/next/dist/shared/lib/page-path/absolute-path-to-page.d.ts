import type { PAGE_TYPES } from '../../../lib/page-types';
/**
 * Given the absolute path to the pages folder, an absolute file path for a
 * page and the page extensions, this function will return the page path
 * relative to the pages folder. It doesn't consider index tail. Example:
 *  - `/Users/rick/my-project/pages/foo/bar/baz.js` -> `/foo/bar/baz`
 *
 * It also handles special metadata routes mapping. Example:
 * - `/Users/rick/my-project/app/sitemap.js` -> `/sitemap/route`
 *
 * @param filepath Absolute path to the page.
 * @param opts.dir Absolute path to the pages/app folder.
 * @param opts.extensions Extensions allowed for the page.
 * @param opts.keepIndex When true the trailing `index` kept in the path.
 * @param opts.pagesType Whether the page is in the pages or app directory.
 */
export declare function absolutePathToPage(pagePath: string, options: {
    extensions: string[] | readonly string[];
    keepIndex: boolean;
    dir: string;
    pagesType: PAGE_TYPES;
}): string;
