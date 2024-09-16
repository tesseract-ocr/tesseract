import { isAppPageRoute } from "../../../lib/is-app-page-route";
import { APP_PATHS_MANIFEST } from "../../../shared/lib/constants";
import { AppNormalizers } from "../normalizers/built/app";
import { RouteKind } from "../route-kind";
import { AppPageRouteMatcher } from "../route-matchers/app-page-route-matcher";
import { ManifestRouteMatcherProvider } from "./manifest-route-matcher-provider";
export class AppPageRouteMatcherProvider extends ManifestRouteMatcherProvider {
    constructor(distDir, manifestLoader){
        super(APP_PATHS_MANIFEST, manifestLoader);
        this.normalizers = new AppNormalizers(distDir);
    }
    async transform(manifest) {
        // This matcher only matches app pages.
        const pages = Object.keys(manifest).filter((page)=>isAppPageRoute(page));
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
            matchers.push(new AppPageRouteMatcher({
                kind: RouteKind.APP_PAGE,
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