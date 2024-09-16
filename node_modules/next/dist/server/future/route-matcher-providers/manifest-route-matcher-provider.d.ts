import type { RouteMatcher } from '../route-matchers/route-matcher';
import type { Manifest, ManifestLoader } from './helpers/manifest-loaders/manifest-loader';
import { CachedRouteMatcherProvider } from './helpers/cached-route-matcher-provider';
export declare abstract class ManifestRouteMatcherProvider<M extends RouteMatcher = RouteMatcher> extends CachedRouteMatcherProvider<M, Manifest | null> {
    constructor(manifestName: string, manifestLoader: ManifestLoader);
}
