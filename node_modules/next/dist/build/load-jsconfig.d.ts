import type { NextConfigComplete } from '../server/config-shared';
export declare function parseJsonFile(filePath: string): any;
export type ResolvedBaseUrl = {
    baseUrl: string;
    isImplicit: boolean;
} | undefined;
export type JsConfig = {
    compilerOptions: Record<string, any>;
} | undefined;
export default function loadJsConfig(dir: string, config: NextConfigComplete): Promise<{
    useTypeScript: boolean;
    jsConfig: JsConfig;
    resolvedBaseUrl: ResolvedBaseUrl;
}>;
