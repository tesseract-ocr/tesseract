import type { FileReader } from './helpers/file-reader/file-reader';
import { AppRouteRouteMatcher } from '../../route-matchers/app-route-route-matcher';
import { FileCacheRouteMatcherProvider } from './file-cache-route-matcher-provider';
export declare class DevAppRouteRouteMatcherProvider extends FileCacheRouteMatcherProvider<AppRouteRouteMatcher> {
    private readonly normalizers;
    constructor(appDir: string, extensions: ReadonlyArray<string>, reader: FileReader);
    protected transform(files: ReadonlyArray<string>): Promise<ReadonlyArray<AppRouteRouteMatcher>>;
}
