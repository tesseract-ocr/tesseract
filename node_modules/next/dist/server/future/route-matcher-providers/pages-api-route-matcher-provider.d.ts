import { PagesAPIRouteMatcher } from '../route-matchers/pages-api-route-matcher';
import type { Manifest, ManifestLoader } from './helpers/manifest-loaders/manifest-loader';
import { ManifestRouteMatcherProvider } from './manifest-route-matcher-provider';
import type { I18NProvider } from '../helpers/i18n-provider';
export declare class PagesAPIRouteMatcherProvider extends ManifestRouteMatcherProvider<PagesAPIRouteMatcher> {
    private readonly i18nProvider?;
    private readonly normalizers;
    constructor(distDir: string, manifestLoader: ManifestLoader, i18nProvider?: I18NProvider | undefined);
    protected transform(manifest: Manifest): Promise<ReadonlyArray<PagesAPIRouteMatcher>>;
}
