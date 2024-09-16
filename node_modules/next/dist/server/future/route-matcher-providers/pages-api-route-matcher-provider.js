"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PagesAPIRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return PagesAPIRouteMatcherProvider;
    }
});
const _isapiroute = require("../../../lib/is-api-route");
const _constants = require("../../../shared/lib/constants");
const _routekind = require("../route-kind");
const _pagesapiroutematcher = require("../route-matchers/pages-api-route-matcher");
const _manifestroutematcherprovider = require("./manifest-route-matcher-provider");
const _pages = require("../normalizers/built/pages");
class PagesAPIRouteMatcherProvider extends _manifestroutematcherprovider.ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader, i18nProvider){
        super(_constants.PAGES_MANIFEST, manifestLoader);
        this.i18nProvider = i18nProvider;
        this.normalizers = new _pages.PagesNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher is only for Pages API routes.
        const pathnames = Object.keys(manifest).filter((pathname)=>(0, _isapiroute.isAPIRoute)(pathname));
        const matchers = [];
        for (const page of pathnames){
            if (this.i18nProvider) {
                // Match the locale on the page name, or default to the default locale.
                const { detectedLocale, pathname } = this.i18nProvider.analyze(page);
                matchers.push(new _pagesapiroutematcher.PagesAPILocaleRouteMatcher({
                    kind: _routekind.RouteKind.PAGES_API,
                    pathname,
                    page,
                    bundlePath: this.normalizers.bundlePath.normalize(page),
                    filename: this.normalizers.filename.normalize(manifest[page]),
                    i18n: {
                        locale: detectedLocale
                    }
                }));
            } else {
                matchers.push(new _pagesapiroutematcher.PagesAPIRouteMatcher({
                    kind: _routekind.RouteKind.PAGES_API,
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