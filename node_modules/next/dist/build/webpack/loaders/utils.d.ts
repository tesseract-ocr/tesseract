import type webpack from 'webpack';
export declare function isActionServerLayerEntryModule(mod: {
    resource: string;
    buildInfo?: any;
}): boolean;
export declare function isClientComponentEntryModule(mod: {
    resource: string;
    buildInfo?: any;
}): any;
export declare const regexCSS: RegExp;
export declare function isCSSMod(mod: {
    resource: string;
    type?: string;
    loaders?: {
        loader: string;
    }[];
}): boolean;
export declare function getActionsFromBuildInfo(mod: {
    resource: string;
    buildInfo?: any;
}): undefined | string[];
export declare function generateActionId(filePath: string, exportName: string): string;
export declare function encodeToBase64<T extends {}>(obj: T): string;
export declare function decodeFromBase64<T extends {}>(str: string): T;
export declare function getLoaderModuleNamedExports(resourcePath: string, context: webpack.LoaderContext<any>): Promise<string[]>;
