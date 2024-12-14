import { type WebpackLayerName } from '../../lib/constants';
import type { NextConfig, ExperimentalConfig } from '../../server/config-shared';
import type { ResolvedBaseUrl } from '../load-jsconfig';
export declare function getParserOptions({ filename, jsConfig, ...rest }: any): any;
export declare function getJestSWCOptions({ isServer, filename, esm, modularizeImports, swcPlugins, compilerOptions, jsConfig, resolvedBaseUrl, pagesDir, serverReferenceHashSalt, }: {
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
    serverReferenceHashSalt: string;
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
        dynamicIoEnabled: boolean | undefined;
    } | undefined;
    serverActions: {
        isReactServerLayer: boolean;
        dynamicIoEnabled: boolean | undefined;
        hashSalt: string;
        cacheKinds: string[];
    } | undefined;
    preferEsm: boolean;
    lintCodemodComments: boolean;
    debugFunctionName: boolean;
    emotion?: {
        importMap?: {
            [importName: string]: {
                [exportName: string]: {
                    canonicalImport?: [string, string];
                    styledBaseImport?: [string, string];
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
        topLevelImportPaths?: string[];
        ssr?: boolean;
        fileName?: boolean;
        meaninglessFileNames?: string[];
        minify?: boolean;
        transpileTemplateLiterals?: boolean;
        namespace?: string;
        pure?: boolean;
        cssProp?: boolean;
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
        exclude?: string[];
    } | undefined;
    reactRemoveProperties: boolean | {
        properties?: string[];
    } | undefined;
    modularizeImports: {
        [k: string]: {
            transform: string | string[][];
            preventFullImport?: boolean;
            skipDefaultConversion?: boolean;
        };
    } | undefined;
    relay: {
        src: string;
        artifactDirectory?: string;
        language?: "typescript" | "javascript" | "flow";
        eagerEsModules?: boolean;
    } | undefined;
    styledJsx: {};
};
export declare function getLoaderSWCOptions({ filename, development, isServer, pagesDir, appDir, isPageFile, isDynamicIo, hasReactRefresh, modularizeImports, optimizeServerReact, optimizePackageImports, swcPlugins, compilerOptions, jsConfig, supportedBrowsers, swcCacheDir, relativeFilePathFromRoot, serverComponents, serverReferenceHashSalt, bundleLayer, esm, cacheHandlers, }: {
    filename: string;
    development: boolean;
    isServer: boolean;
    pagesDir?: string;
    appDir?: string;
    isPageFile: boolean;
    hasReactRefresh: boolean;
    optimizeServerReact?: boolean;
    modularizeImports: NextConfig['modularizeImports'];
    isDynamicIo?: boolean;
    optimizePackageImports?: NonNullable<NextConfig['experimental']>['optimizePackageImports'];
    swcPlugins: ExperimentalConfig['swcPlugins'];
    compilerOptions: NextConfig['compiler'];
    jsConfig: any;
    supportedBrowsers: string[] | undefined;
    swcCacheDir: string;
    relativeFilePathFromRoot: string;
    esm?: boolean;
    serverComponents?: boolean;
    serverReferenceHashSalt: string;
    bundleLayer?: WebpackLayerName;
    cacheHandlers: ExperimentalConfig['cacheHandlers'];
}): any;
