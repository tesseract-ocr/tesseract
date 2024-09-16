export type BuildManifest = {
    devFiles: readonly string[];
    ampDevFiles: readonly string[];
    polyfillFiles: readonly string[];
    lowPriorityFiles: readonly string[];
    rootMainFiles: readonly string[];
    pages: {
        '/_app': readonly string[];
        [page: string]: readonly string[];
    };
    ampFirstPages: readonly string[];
};
export declare function getPageFiles(buildManifest: BuildManifest, page: string): readonly string[];
