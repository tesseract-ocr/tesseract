import type { NextConfigComplete } from '../server/config-shared';
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
