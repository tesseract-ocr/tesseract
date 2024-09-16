"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PagesRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return PagesRouteMatcherProvider;
    }
});
const _isapiroute = require("../../../lib/is-api-route");
const _constants = require("../../../shared/lib/constants");
const _routekind = require("../route-kind");
const _pagesroutematcher = require("../route-matchers/pages-route-matcher");
const _manifestroutematcherprovider = require("./manifest-route-matcher-provider");
const _pages = require("../normalizers/built/pages");
class PagesRouteMatcherProvider extends _manifestroutematcherprovider.ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader, i18nProvider){
        super(_constants.PAGES_MANIFEST, manifestLoader);
        this.i18nProvider = i18nProvider;
        this.normalizers = new _pages.PagesNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher is only for Pages routes, not Pages API routes which are
        // included in this manifest.
        const pathnames = Object.keys(manifest).filter((pathname)=>!(0, _isapiroute.isAPIRoute)(pathname))// Remove any blocked pages (page that can't be routed to, like error or
        // internal pages).
        .filter((pathname)=>{
            var _this_i18nProvider;
            const normalized = ((_this_i18nProvider = this.i18nProvider) == null ? void 0 : _this_i18nProvider.analyze(pathname).pathname) ?? pathname;
            // Skip any blocked pages.
            if (_constants.BLOCKED_PAGES.includes(normalized)) return false;
            return true;
        });
        const matchers = [];
        for (const page of pathnames){
            if (this.i18nProvider) {
                // Match the locale on the page name, or default to the default locale.
                const { detectedLocale, pathname } = this.i18nProvider.analyze(page);
                matchers.push(new _pagesroutematcher.PagesLocaleRouteMatcher({
                    kind: _routekind.RouteKind.PAGES,
                    pathname,
                    page,
                    bundlePath: this.normalizers.bundlePath.normalize(page),
                    filename: this.normalizers.filename.normalize(manifest[page]),
                    i18n: {
                        locale: detectedLocale
                    }
                }));
            } else {
                matchers.push(new _pagesroutematcher.PagesRouteMatcher({
                    kind: _routekind.RouteKind.PAGES,
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