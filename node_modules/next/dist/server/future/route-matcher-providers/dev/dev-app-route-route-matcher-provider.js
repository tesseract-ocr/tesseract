"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DevAppRouteRouteMatcherProvider", {
    enumerable: true,
    get: function() {
        return DevAppRouteRouteMatcherProvider;
    }
});
const _approuteroutematcher = require("../../route-matchers/app-route-route-matcher");
const _routekind = require("../../route-kind");
const _filecacheroutematcherprovider = require("./file-cache-route-matcher-provider");
const _isapprouteroute = require("../../../../lib/is-app-route-route");
const _app = require("../../normalizers/built/app");
class DevAppRouteRouteMatcherProvider extends _filecacheroutematcherprovider.FileCacheRouteMatcherProvider {
    constructor(appDir, extensions, reader){
        super(appDir, reader);
        this.normalizers = new _app.DevAppNormalizers(appDir, extensions);
    }
    async transform(files) {
        const matchers = [];
        for (const filename of files){
            const page = this.normalizers.page.normalize(filename);
            // If the file isn't a match for this matcher, then skip it.
            if (!(0, _isapprouteroute.isAppRouteRoute)(page)) continue;
            // Validate that this is not an ignored page.
            if (page.includes("/_")) continue;
            const pathname = this.normalizers.pathname.normalize(filename);
            const bundlePath = this.normalizers.bundlePath.normalize(filename);
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

//# sourceMappingURL=dev-app-route-route-matcher-provider.js.map