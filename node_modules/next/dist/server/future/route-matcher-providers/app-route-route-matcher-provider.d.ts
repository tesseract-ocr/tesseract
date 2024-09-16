import { AppRouteRouteMatcher } from '../route-matchers/app-route-route-matcher';
import type { Manifest, ManifestLoader } from './helpers/manifest-loaders/manifest-loader';
import { ManifestRouteMatcherProvider } from './manifest-route-matcher-provider';
export declare class AppRouteRouteMatcherProvider extends ManifestRouteMatcherProvider<AppRouteRouteMatcher> {
    private readonly normalizers;
    constructor(distDir: string, manifestLoader: ManifestLoader);
    protected transform(manifest: Manifest): Promise<ReadonlyArray<AppRouteRouteMatcher>>;
}
