import type { RouteMatcherProvider } from '../route-matcher-provider';
import type { RouteMatcher } from '../../route-matchers/route-matcher';
interface LoaderComparable<D> {
    load(): Promise<D>;
    compare(left: D, right: D): boolean;
}
/**
 * This will memoize the matchers if the loaded data is comparable.
 */
export declare abstract class CachedRouteMatcherProvider<M extends RouteMatcher = RouteMatcher, D = any> implements RouteMatcherProvider<M> {
    private readonly loader;
    private data?;
    private cached;
    constructor(loader: LoaderComparable<D>);
    protected abstract transform(data: D): Promise<ReadonlyArray<M>>;
    matchers(): Promise<readonly M[]>;
}
export {};
