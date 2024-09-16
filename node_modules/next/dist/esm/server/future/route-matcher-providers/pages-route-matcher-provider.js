import { isAPIRoute } from "../../../lib/is-api-route";
import { BLOCKED_PAGES, PAGES_MANIFEST } from "../../../shared/lib/constants";
import { RouteKind } from "../route-kind";
import { PagesLocaleRouteMatcher, PagesRouteMatcher } from "../route-matchers/pages-route-matcher";
import { ManifestRouteMatcherProvider } from "./manifest-route-matcher-provider";
import { PagesNormalizers } from "../normalizers/built/pages";
export class PagesRouteMatcherProvider extends ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader, i18nProvider){
        super(PAGES_MANIFEST, manifestLoader);
        this.i18nProvider = i18nProvider;
        this.normalizers = new PagesNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher is only for Pages routes, not Pages API routes which are
        // included in this manifest.
        const pathnames = Object.keys(manifest).filter((pathname)=>!isAPIRoute(pathname))// Remove any blocked pages (page that can't be routed to, like error or
        // internal pages).
        .filter((pathname)=>{
            var _this_i18nProvider;
            const normalized = ((_this_i18nProvider = this.i18nProvider) == null ? void 0 : _this_i18nProvider.analyze(pathname).pathname) ?? pathname;
            // Skip any blocked pages.
            if (BLOCKED_PAGES.includes(normalized)) return false;
            return true;
        });
        const matchers = [];
        for (const page of pathnames){
            if (this.i18nProvider) {
                // Match the locale on the page name, or default to the default locale.
                const { detectedLocale, pathname } = this.i18nProvider.analyze(page);
                matchers.push(new PagesLocaleRouteMatcher({
                    kind: RouteKind.PAGES,
                    pathname,
                    page,
                    bundlePath: this.normalizers.bundlePath.normalize(page),
                    filename: this.normalizers.filename.normalize(manifest[page]),
                    i18n: {
                        locale: detectedLocale
                    }
                }));
            } else {
                matchers.push(new PagesRouteMatcher({
                    kind: RouteKind.PAGES,
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

//# sourceMappingURL=pages-route-matcher-provider.js.map