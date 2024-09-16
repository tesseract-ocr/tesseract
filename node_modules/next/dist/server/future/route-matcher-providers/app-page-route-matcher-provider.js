"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AppPageRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return AppPageRouteMatcherProvider;
    }
});
const _isapppageroute = require("../../../lib/is-app-page-route");
const _constants = require("../../../shared/lib/constants");
const _app = require("../normalizers/built/app");
const _routekind = require("../route-kind");
const _apppageroutematcher = require("../route-matchers/app-page-route-matcher");
const _manifestroutematcherprovider = require("./manifest-route-matcher-provider");
class AppPageRouteMatcherProvider extends _manifestroutematcherprovider.ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader){
        super(_constants.APP_PATHS_MANIFEST, manifestLoader);
        this.normalizers = new _app.AppNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher only matches app pages.
        const pages = Object.keys(manifest).filter((page)=>(0, _isapppageroute.isAppPageRoute)(page));
        // Collect all the app paths for each page. This could include any parallel
        // routes.
        const allAppPaths = {};
        for (const page of pages){
            const pathname = this.normalizers.pathname.normalize(page);
            if (pathname in allAppPaths) allAppPaths[pathname].push(page);
            else allAppPaths[pathname] = [
                page
            ];
        }
        // Format the routes.
        const matchers = [];
        for (const [pathname, appPaths] of Object.entries(allAppPaths)){
            // TODO-APP: (wyattjoh) this is a hack right now, should be more deterministic
            const page = appPaths[0];
            const filename = this.normalizers.filename.normalize(manifest[page]);
            const bundlePath = this.normalizers.bundlePath.normalize(page);
            matchers.push(new _apppageroutematcher.AppPageRouteMatcher({
                kind: _routekind.RouteKind.APP_PAGE,
                pathname,
                page,
                bundlePath,
                filename,
                appPaths
            }));
        }
        return matchers;
    }
}

//# sourceMappingURL=app-page-route-matcher-provider.js.map