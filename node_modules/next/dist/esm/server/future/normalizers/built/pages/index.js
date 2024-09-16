import { DevPagesBundlePathNormalizer, PagesBundlePathNormalizer } from "./pages-bundle-path-normalizer";
import { PagesFilenameNormalizer } from "./pages-filename-normalizer";
import { DevPagesPageNormalizer } from "./pages-page-normalizer";
import { DevPagesPathnameNormalizer } from "./pages-pathname-normalizer";
export class PagesNormalizers {
    constructor(distDir){
        this.filename = new PagesFilenameNormalizer(distDir);
        this.bundlePath = new PagesBundlePathNormalizer();
    // You'd think that we'd require a `pathname` normalizer here, but for
    // `/pages` we have to handle i18n routes, which means that we need to
    // analyze the page path to determine the locale prefix and it's locale.
    }
}
export class DevPagesNormalizers {
    constructor(pagesDir, extensions){
        this.page = new DevPagesPageNormalizer(pagesDir, extensions);
        this.pathname = new DevPagesPathnameNormalizer(pagesDir, extensions);
        this.bundlePath = new DevPagesBundlePathNormalizer(this.page);
    }
}

//# sourceMappingURL=index.js.map