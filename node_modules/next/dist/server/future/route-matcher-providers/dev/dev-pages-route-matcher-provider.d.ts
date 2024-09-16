import type { FileReader } from './helpers/file-reader/file-reader';
import { PagesRouteMatcher } from '../../route-matchers/pages-route-matcher';
import type { LocaleRouteNormalizer } from '../../normalizers/locale-route-normalizer';
import { FileCacheRouteMatcherProvider } from './file-cache-route-matcher-provider';
export declare class DevPagesRouteMatcherProvider extends FileCacheRouteMatcherProvider<PagesRouteMatcher> {
    private readonly pagesDir;
    private readonly extensions;
    private readonly localeNormalizer?;
    private readonly expression;
    private readonly normalizers;
    constructor(pagesDir: string, extensions: ReadonlyArray<string>, reader: FileReader, localeNormalizer?: LocaleRouteNormalizer | undefined);
    private test;
    protected transform(files: ReadonlyArray<string>): Promise<ReadonlyArray<PagesRouteMatcher>>;
}
