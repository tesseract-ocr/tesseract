import { DevPagesBundlePathNormalizer, PagesBundlePathNormalizer } from './pages-bundle-path-normalizer';
import { PagesFilenameNormalizer } from './pages-filename-normalizer';
import { DevPagesPageNormalizer } from './pages-page-normalizer';
import { DevPagesPathnameNormalizer } from './pages-pathname-normalizer';
export declare class PagesNormalizers {
    readonly filename: PagesFilenameNormalizer;
    readonly bundlePath: PagesBundlePathNormalizer;
    constructor(distDir: string);
}
export declare class DevPagesNormalizers {
    readonly page: DevPagesPageNormalizer;
    readonly pathname: DevPagesPathnameNormalizer;
    readonly bundlePath: DevPagesBundlePathNormalizer;
    constructor(pagesDir: string, extensions: ReadonlyArray<string>);
}
