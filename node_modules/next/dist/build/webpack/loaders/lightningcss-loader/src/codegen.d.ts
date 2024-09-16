import type { LoaderContext } from 'next/dist/compiled/webpack/webpack';
export interface CssImport {
    icss?: boolean;
    importName: string;
    url: string;
    type?: 'url' | string;
    index?: number;
}
export interface CssExport {
    name: string;
    value: string;
}
export interface ApiParam {
    url?: string;
    importName?: string;
    layer?: string;
    supports?: string;
    media?: string;
    dedupe?: boolean;
    index?: number;
}
export interface ApiReplacement {
    replacementName: string;
    localName?: string;
    importName: string;
    needQuotes?: boolean;
    hash?: string;
}
export declare function getImportCode(imports: CssImport[], options: any): string;
export declare function getModuleCode(result: {
    map: any;
    css: any;
}, api: ApiParam[], replacements: ApiReplacement[], options: any, loaderContext: LoaderContext<any>): string;
export declare function getExportCode(exports: CssExport[], replacements: ApiReplacement[], options: any): string;
