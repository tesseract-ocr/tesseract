import { AppBundlePathNormalizer, DevAppBundlePathNormalizer } from './app-bundle-path-normalizer';
import { AppFilenameNormalizer } from './app-filename-normalizer';
import { DevAppPageNormalizer } from './app-page-normalizer';
import { AppPathnameNormalizer, DevAppPathnameNormalizer } from './app-pathname-normalizer';
export declare class AppNormalizers {
    readonly filename: AppFilenameNormalizer;
    readonly pathname: AppPathnameNormalizer;
    readonly bundlePath: AppBundlePathNormalizer;
    constructor(distDir: string);
}
export declare class DevAppNormalizers {
    readonly page: DevAppPageNormalizer;
    readonly pathname: DevAppPathnameNormalizer;
    readonly bundlePath: DevAppBundlePathNormalizer;
    constructor(appDir: string, extensions: ReadonlyArray<string>);
}
