"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AppRouteRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return AppRouteRouteMatcherProvider;
    }
});
const _isapprouteroute = require("../../../lib/is-app-route-route");
const _constants = require("../../../shared/lib/constants");
const _routekind = require("../route-kind");
const _approuteroutematcher = require("../route-matchers/app-route-route-matcher");
const _manifestroutematcherprovider = require("./manifest-route-matcher-provider");
const _app = require("../normalizers/built/app");
class AppRouteRouteMatcherProvider extends _manifestroutematcherprovider.ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader){
        super(_constants.APP_PATHS_MANIFEST, manifestLoader);
        this.normalizers = new _app.AppNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher only matches app routes.
        const pages = Object.keys(manifest).filter((page)=>(0, _isapprouteroute.isAppRouteRoute)(page));
        // Format the routes.
        const matchers = [];
        for (const page of pages){
            const filename = this.normalizers.filename.normalize(manifest[page]);
            const pathname = this.normalizers.pathname.normalize(page);
            const bundlePath = this.normalizers.bundlePath.normalize(page);
            matchers.push(new _approuteroutematcher.AppRouteRouteMatcher({
                kind: _routekind.RouteKind.APP_ROUTE,
                pathname,
                page,
                bundlePath,
                filename
            }));
        }
        return matchers;
    }
}

//# sourceMappingURL=app-route-route-matcher-provider.js.map