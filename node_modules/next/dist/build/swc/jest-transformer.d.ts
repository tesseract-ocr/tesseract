import type { Config } from '@jest/types';
import type { NextConfig, ExperimentalConfig } from '../../server/config-shared';
import type { ResolvedBaseUrl } from '../load-jsconfig';
type TransformerConfig = Config.TransformerConfig[1];
export interface JestTransformerConfig extends TransformerConfig {
    jsConfig: any;
    resolvedBaseUrl?: ResolvedBaseUrl;
    pagesDir?: string;
    serverComponents?: boolean;
    isEsmProject: boolean;
    modularizeImports?: NextConfig['modularizeImports'];
    swcPlugins: ExperimentalConfig['swcPlugins'];
    compilerOptions: NextConfig['compiler'];
}
export {};
