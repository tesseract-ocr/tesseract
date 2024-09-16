/**
 * Calculate all possible pagePaths for a given normalized pagePath along with
 * allowed extensions. This can be used to check which one of the files exists
 * and to debug inspected locations.
 *
 * For pages, map `/route` to [`/route.[ext]`, `/route/index.[ext]`]
 * For app paths, map `/route/page` to [`/route/page.[ext]`] or `/route/route`
 * to [`/route/route.[ext]`]
 *
 * @param normalizedPagePath Normalized page path (it will denormalize).
 * @param extensions Allowed extensions.
 */
export declare function getPagePaths(normalizedPagePath: string, extensions: string[], isAppDir: boolean): string[];
