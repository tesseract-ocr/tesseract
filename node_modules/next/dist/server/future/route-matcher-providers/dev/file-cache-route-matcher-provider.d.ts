import type { RouteMatcher } from '../../route-matchers/route-matcher';
import { CachedRouteMatcherProvider } from '../helpers/cached-route-matcher-provider';
import type { FileReader } from './helpers/file-reader/file-reader';
/**
 * This will memoize the matchers when the file contents are the same.
 */
export declare abstract class FileCacheRouteMatcherProvider<M extends RouteMatcher = RouteMatcher> extends CachedRouteMatcherProvider<M, ReadonlyArray<string>> {
    constructor(dir: string, reader: FileReader);
}
