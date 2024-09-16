import { AppRouteRouteMatcher } from "../../route-matchers/app-route-route-matcher";
import { RouteKind } from "../../route-kind";
import { FileCacheRouteMatcherProvider } from "./file-cache-route-matcher-provider";
import { isAppRouteRoute } from "../../../../lib/is-app-route-route";
import { DevAppNormalizers } from "../../normalizers/built/app";
export class DevAppRouteRouteMatcherProvider extends FileCacheRouteMatcherProvider {
    constructor(appDir, extensions, reader){
        super(appDir, reader);
        this.normalizers = new DevAppNormalizers(appDir, extensions);
    }
    async transform(files) {
        const matchers = [];
        for (const filename of files){
            const page = this.normalizers.page.normalize(filename);
            // If the file isn't a match for this matcher, then skip it.
            if (!isAppRouteRoute(page)) continue;
            // Validate that this is not an ignored page.
            if (page.includes("/_")) continue;
            const pathname = this.normalizers.pathname.normalize(filename);
            const bundlePath = this.normalizers.bundlePath.normalize(filename);
            matchers.push(new AppRouteRouteMatcher({
                kind: RouteKind.APP_ROUTE,
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