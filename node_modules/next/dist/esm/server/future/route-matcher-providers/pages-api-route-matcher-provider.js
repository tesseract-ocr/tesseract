import { isAPIRoute } from "../../../lib/is-api-route";
import { PAGES_MANIFEST } from "../../../shared/lib/constants";
import { RouteKind } from "../route-kind";
import { PagesAPILocaleRouteMatcher, PagesAPIRouteMatcher } from "../route-matchers/pages-api-route-matcher";
import { ManifestRouteMatcherProvider } from "./manifest-route-matcher-provider";
import { PagesNormalizers } from "../normalizers/built/pages";
export class PagesAPIRouteMatcherProvider extends ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader, i18nProvider){
        super(PAGES_MANIFEST, manifestLoader);
        this.i18nProvider = i18nProvider;
        this.normalizers = new PagesNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher is only for Pages API routes.
        const pathnames = Object.keys(manifest).filter((pathname)=>isAPIRoute(pathname));
        const matchers = [];
        for (const page of pathnames){
            if (this.i18nProvider) {
                // Match the locale on the page name, or default to the default locale.
                const { detectedLocale, pathname } = this.i18nProvider.analyze(page);
                matchers.push(new PagesAPILocaleRouteMatcher({
                    kind: RouteKind.PAGES_API,
                    pathname,
                    page,
                    bundlePath: this.normalizers.bundlePath.normalize(page),
                    filename: this.normalizers.filename.normalize(manifest[page]),
                    i18n: {
                        locale: detectedLocale
                    }
                }));
            } else {
                matchers.push(new PagesAPIRouteMatcher({
                    kind: RouteKind.PAGES_API,
                    // In `pages/`, the page is the same as the pathname.
                    pathname: page,
                    page,
                    bundlePath: this.normalizers.bundlePath.normalize(page),
                    filename: this.normalizers.filename.normalize(manifest[page])
                }));
            }
        }
        return matchers;
    }
}

//# sourceMappingURL=pages-api-route-matcher-provider.js.map