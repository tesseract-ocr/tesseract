import { AppBundlePathNormalizer, DevAppBundlePathNormalizer } from "./app-bundle-path-normalizer";
import { AppFilenameNormalizer } from "./app-filename-normalizer";
import { DevAppPageNormalizer } from "./app-page-normalizer";
import { AppPathnameNormalizer, DevAppPathnameNormalizer } from "./app-pathname-normalizer";
export class AppNormalizers {
    constructor(distDir){
        this.filename = new AppFilenameNormalizer(distDir);
        this.pathname = new AppPathnameNormalizer();
        this.bundlePath = new AppBundlePathNormalizer();
    }
}
export class DevAppNormalizers {
    constructor(appDir, extensions){
        this.page = new DevAppPageNormalizer(appDir, extensions);
        this.pathname = new DevAppPathnameNormalizer(this.page);
        this.bundlePath = new DevAppBundlePathNormalizer(this.page);
    }
}

//# sourceMappingURL=index.js.map