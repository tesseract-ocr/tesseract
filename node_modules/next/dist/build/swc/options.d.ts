import { type WebpackLayerName } from '../../lib/constants';
import type { NextConfig, ExperimentalConfig } from '../../server/config-shared';
import type { ResolvedBaseUrl } from '../load-jsconfig';
export declare function getParserOptions({ filename, jsConfig, ...rest }: any): any;
export declare function getJestSWCOptions({ isServer, filename, esm, modularizeImports, swcPlugins, compilerOptions, jsConfig, resolvedBaseUrl, pagesDir, }: {
    isServer: boolean;
    filename: string;
    esm: boolean;
    modularizeImports?: NextConfig['modularizeImports'];
    swcPlugins: ExperimentalConfig['swcPlugins'];
    compilerOptions: NextConfig['compiler'];
    jsConfig: any;
    resolvedBaseUrl?: ResolvedBaseUrl;
    pagesDir?: string;
    serverComponents?: boolean;
}): {
    env: {
        targets: {
            node: string;
        };
    };
    module: {
        type: string;
    };
    disableNextSsg: boolean;
    disablePageConfig: boolean;
    pagesDir: string | undefined;
    serverComponents: {
        isReactServerLayer: boolean;
    } | undefined;
    serverActions: {
        enabled: boolean;
        isReactServerLayer: boolean;
        hashSalt: string;
    } | undefined;
    preferEsm: boolean;
    emotion?: {
        importMap?: {
            [importName: string]: {
                [exportName: string]: {
                    canonicalImport?: [string, string] | undefined;
                    styledBaseImport?: [string, string] | undefined;
                };
            };
        } | undefined;
        labelFormat?: string | undefined;
        sourcemap: boolean;
        enabled: boolean;
        autoLabel: boolean;
    } | null | undefined;
    styledComponents?: {
        displayName: boolean;
        topLevelImportPaths?: string[] | undefined;
        ssr?: boolean | undefined;
        fileName?: boolean | undefined;
        meaninglessFileNames?: string[] | undefined;
        minify?: boolean | undefined;
        transpileTemplateLiterals?: boolean | undefined;
        namespace?: string | undefined;
        pure?: boolean | undefined;
        cssProp?: boolean | undefined;
    } | null | undefined;
    jsc: {
        externalHelpers: boolean;
        parser: any;
        experimental: {
            keepImportAttributes: boolean;
            emitAssertForImportAttributes: boolean;
            plugins: any[][];
            cacheRoot: string | undefined;
        };
        transform: {
            legacyDecorator: boolean;
            decoratorMetadata: boolean;
            useDefineForClassFields: boolean;
            react: {
                importSource: any;
                runtime: string;
                pragmaFrag: string;
                throwIfNamespace: boolean;
                development: boolean;
                useBuiltins: boolean;
                refresh: boolean;
            };
            optimizer: {
                simplify: boolean;
                globals: {
                    typeofs: {
                        window: string;
                    };
                    envs: {
                        NODE_ENV: string;
                    };
                } | null;
            };
            regenerator: {
                importPath: string;
            };
            hidden?: {
                jest: boolean;
            } | undefined;
        };
        baseUrl?: string | undefined;
        paths?: any;
    };
    sourceMaps: string | undefined;
    removeConsole: boolean | {
        exclude?: string[] | undefined;
    } | undefined;
    reactRemoveProperties: boolean | {
        properties?: string[] | undefined;
    } | undefined;
    modularizeImports: {
        [k: string]: {
            transform: string | string[][];
            preventFullImport?: boolean | undefined;
            skipDefaultConversion?: boolean | undefined;
        };
    } | undefined;
    relay: {
        src: string;
        artifactDirectory?: string | undefined;
        language?: "flow" | "typescript" | "javascript" | undefined;
        eagerEsModules?: boolean | undefined;
    } | undefined;
    styledJsx: {};
};
export declare function getLoaderSWCOptions({ filename, development, isServer, pagesDir, appDir, isPageFile, hasReactRefresh, modularizeImports, optimizeServerReact, optimizePackageImports, swcPlugins, compilerOptions, jsConfig, supportedBrowsers, swcCacheDir, relativeFilePathFromRoot, serverComponents, bundleLayer, esm, }: {
    filename: string;
    development: boolean;
    isServer: boolean;
    pagesDir?: string;
    appDir?: string;
    isPageFile: boolean;
    hasReactRefresh: boolean;
    optimizeServerReact?: boolean;
    modularizeImports: NextConfig['modularizeImports'];
    optimizePackageImports?: NonNullable<NextConfig['experimental']>['optimizePackageImports'];
    swcPlugins: ExperimentalConfig['swcPlugins'];
    compilerOptions: NextConfig['compiler'];
    jsConfig: any;
    supportedBrowsers: string[] | undefined;
    swcCacheDir: string;
    relativeFilePathFromRoot: string;
    esm?: boolean;
    serverComponents?: boolean;
    bundleLayer?: WebpackLayerName;
}): any;
