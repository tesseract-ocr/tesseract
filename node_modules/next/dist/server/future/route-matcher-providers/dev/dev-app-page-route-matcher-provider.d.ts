import type { FileReader } from './helpers/file-reader/file-reader';
import { AppPageRouteMatcher } from '../../route-matchers/app-page-route-matcher';
import { FileCacheRouteMatcherProvider } from './file-cache-route-matcher-provider';
export declare class DevAppPageRouteMatcherProvider extends FileCacheRouteMatcherProvider<AppPageRouteMatcher> {
    private readonly expression;
    private readonly normalizers;
    constructor(appDir: string, extensions: ReadonlyArray<string>, reader: FileReader);
    protected transform(files: ReadonlyArray<string>): Promise<ReadonlyArray<AppPageRouteMatcher>>;
}
