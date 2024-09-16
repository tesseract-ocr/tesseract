declare function normalizeUrl(url: string, isStringValue: boolean): string;
declare function requestify(url: string, rootContext: string): any;
declare function getFilter(filter: any, resourcePath: string): (...args: any[]) => any;
declare function shouldUseImportPlugin(options: any): any;
declare function shouldUseURLPlugin(options: any): any;
declare function shouldUseModulesPlugins(options: any): boolean;
declare function shouldUseIcssPlugin(options: any): boolean;
declare function getModulesPlugins(options: any, loaderContext: any, meta: any): any[];
declare function normalizeSourceMap(map: any, resourcePath: string): any;
declare function getPreRequester({ loaders, loaderIndex }: any): (number: any) => any;
declare function getImportCode(imports: any, options: any): string;
declare function normalizeSourceMapForRuntime(map: any, loaderContext: any): string;
declare function getModuleCode(result: {
    map: any;
    css: any;
}, api: any, replacements: any, options: {
    modules: {
        exportOnlyLocals: boolean;
        namedExport: any;
    };
    sourceMap: any;
}, loaderContext: any): string;
declare function dashesCamelCase(str: string): string;
declare function getExportCode(exports: any, replacements: any, options: {
    modules: {
        namedExport: any;
        exportLocalsConvention: any;
        exportOnlyLocals: any;
    };
    esModule: any;
}): string;
declare function resolveRequests(resolve: (arg0: any, arg1: any) => Promise<any>, context: any, possibleRequests: any[]): Promise<any>;
declare function isUrlRequestable(url: string): boolean;
declare function sort(a: {
    index: number;
}, b: {
    index: number;
}): number;
declare function isDataUrl(url: string): boolean;
export { isDataUrl, shouldUseModulesPlugins, shouldUseImportPlugin, shouldUseURLPlugin, shouldUseIcssPlugin, normalizeUrl, requestify, getFilter, getModulesPlugins, normalizeSourceMap, getPreRequester, getImportCode, getModuleCode, getExportCode, resolveRequests, isUrlRequestable, sort, normalizeSourceMapForRuntime, dashesCamelCase, };
