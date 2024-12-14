"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "configSchema", {
    enumerable: true,
    get: function() {
        return configSchema;
    }
});
const _imageconfig = require("../shared/lib/image-config");
const _zod = require("next/dist/compiled/zod");
const _nexttest = require("../cli/next-test");
// A custom zod schema for the SizeLimit type
const zSizeLimit = _zod.z.custom((val)=>{
    if (typeof val === 'number' || typeof val === 'string') {
        return true;
    }
    return false;
});
const zExportMap = _zod.z.record(_zod.z.string(), _zod.z.object({
    page: _zod.z.string(),
    query: _zod.z.any(),
    // private optional properties
    _fallbackRouteParams: _zod.z.array(_zod.z.string()).optional(),
    _isAppDir: _zod.z.boolean().optional(),
    _isDynamicError: _zod.z.boolean().optional(),
    _isRoutePPREnabled: _zod.z.boolean().optional(),
    _isProspectiveRender: _zod.z.boolean().optional()
}));
const zRouteHas = _zod.z.union([
    _zod.z.object({
        type: _zod.z.enum([
            'header',
            'query',
            'cookie'
        ]),
        key: _zod.z.string(),
        value: _zod.z.string().optional()
    }),
    _zod.z.object({
        type: _zod.z.literal('host'),
        key: _zod.z.undefined().optional(),
        value: _zod.z.string()
    })
]);
const zRewrite = _zod.z.object({
    source: _zod.z.string(),
    destination: _zod.z.string(),
    basePath: _zod.z.literal(false).optional(),
    locale: _zod.z.literal(false).optional(),
    has: _zod.z.array(zRouteHas).optional(),
    missing: _zod.z.array(zRouteHas).optional(),
    internal: _zod.z.boolean().optional()
});
const zRedirect = _zod.z.object({
    source: _zod.z.string(),
    destination: _zod.z.string(),
    basePath: _zod.z.literal(false).optional(),
    locale: _zod.z.literal(false).optional(),
    has: _zod.z.array(zRouteHas).optional(),
    missing: _zod.z.array(zRouteHas).optional(),
    internal: _zod.z.boolean().optional()
}).and(_zod.z.union([
    _zod.z.object({
        statusCode: _zod.z.never().optional(),
        permanent: _zod.z.boolean()
    }),
    _zod.z.object({
        statusCode: _zod.z.number(),
        permanent: _zod.z.never().optional()
    })
]));
const zHeader = _zod.z.object({
    source: _zod.z.string(),
    basePath: _zod.z.literal(false).optional(),
    locale: _zod.z.literal(false).optional(),
    headers: _zod.z.array(_zod.z.object({
        key: _zod.z.string(),
        value: _zod.z.string()
    })),
    has: _zod.z.array(zRouteHas).optional(),
    missing: _zod.z.array(zRouteHas).optional(),
    internal: _zod.z.boolean().optional()
});
const zTurboLoaderItem = _zod.z.union([
    _zod.z.string(),
    _zod.z.object({
        loader: _zod.z.string(),
        // Any JSON value can be used as turbo loader options, so use z.any() here
        options: _zod.z.record(_zod.z.string(), _zod.z.any())
    })
]);
const zTurboRuleConfigItemOptions = _zod.z.object({
    loaders: _zod.z.array(zTurboLoaderItem),
    as: _zod.z.string().optional()
});
const zTurboRuleConfigItem = _zod.z.union([
    _zod.z.literal(false),
    _zod.z.record(_zod.z.string(), _zod.z.lazy(()=>zTurboRuleConfigItem)),
    zTurboRuleConfigItemOptions
]);
const zTurboRuleConfigItemOrShortcut = _zod.z.union([
    _zod.z.array(zTurboLoaderItem),
    zTurboRuleConfigItem
]);
const configSchema = _zod.z.lazy(()=>_zod.z.strictObject({
        amp: _zod.z.object({
            canonicalBase: _zod.z.string().optional()
        }).optional(),
        assetPrefix: _zod.z.string().optional(),
        basePath: _zod.z.string().optional(),
        bundlePagesRouterDependencies: _zod.z.boolean().optional(),
        cacheHandler: _zod.z.string().min(1).optional(),
        cacheMaxMemorySize: _zod.z.number().optional(),
        cleanDistDir: _zod.z.boolean().optional(),
        compiler: _zod.z.strictObject({
            emotion: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.object({
                    sourceMap: _zod.z.boolean().optional(),
                    autoLabel: _zod.z.union([
                        _zod.z.literal('always'),
                        _zod.z.literal('dev-only'),
                        _zod.z.literal('never')
                    ]).optional(),
                    labelFormat: _zod.z.string().min(1).optional(),
                    importMap: _zod.z.record(_zod.z.string(), _zod.z.record(_zod.z.string(), _zod.z.object({
                        canonicalImport: _zod.z.tuple([
                            _zod.z.string(),
                            _zod.z.string()
                        ]).optional(),
                        styledBaseImport: _zod.z.tuple([
                            _zod.z.string(),
                            _zod.z.string()
                        ]).optional()
                    }))).optional()
                })
            ]).optional(),
            reactRemoveProperties: _zod.z.union([
                _zod.z.boolean().optional(),
                _zod.z.object({
                    properties: _zod.z.array(_zod.z.string()).optional()
                })
            ]).optional(),
            relay: _zod.z.object({
                src: _zod.z.string(),
                artifactDirectory: _zod.z.string().optional(),
                language: _zod.z.enum([
                    'javascript',
                    'typescript',
                    'flow'
                ]).optional(),
                eagerEsModules: _zod.z.boolean().optional()
            }).optional(),
            removeConsole: _zod.z.union([
                _zod.z.boolean().optional(),
                _zod.z.object({
                    exclude: _zod.z.array(_zod.z.string()).min(1).optional()
                })
            ]).optional(),
            styledComponents: _zod.z.union([
                _zod.z.boolean().optional(),
                _zod.z.object({
                    displayName: _zod.z.boolean().optional(),
                    topLevelImportPaths: _zod.z.array(_zod.z.string()).optional(),
                    ssr: _zod.z.boolean().optional(),
                    fileName: _zod.z.boolean().optional(),
                    meaninglessFileNames: _zod.z.array(_zod.z.string()).optional(),
                    minify: _zod.z.boolean().optional(),
                    transpileTemplateLiterals: _zod.z.boolean().optional(),
                    namespace: _zod.z.string().min(1).optional(),
                    pure: _zod.z.boolean().optional(),
                    cssProp: _zod.z.boolean().optional()
                })
            ]),
            styledJsx: _zod.z.union([
                _zod.z.boolean().optional(),
                _zod.z.object({
                    useLightningcss: _zod.z.boolean().optional()
                })
            ]),
            define: _zod.z.record(_zod.z.string(), _zod.z.string()).optional()
        }).optional(),
        compress: _zod.z.boolean().optional(),
        configOrigin: _zod.z.string().optional(),
        crossOrigin: _zod.z.union([
            _zod.z.literal('anonymous'),
            _zod.z.literal('use-credentials')
        ]).optional(),
        deploymentId: _zod.z.string().optional(),
        devIndicators: _zod.z.object({
            appIsrStatus: _zod.z.boolean().optional(),
            buildActivity: _zod.z.boolean().optional(),
            buildActivityPosition: _zod.z.union([
                _zod.z.literal('bottom-left'),
                _zod.z.literal('bottom-right'),
                _zod.z.literal('top-left'),
                _zod.z.literal('top-right')
            ]).optional()
        }).optional(),
        distDir: _zod.z.string().min(1).optional(),
        env: _zod.z.record(_zod.z.string(), _zod.z.union([
            _zod.z.string(),
            _zod.z.undefined()
        ])).optional(),
        eslint: _zod.z.strictObject({
            dirs: _zod.z.array(_zod.z.string().min(1)).optional(),
            ignoreDuringBuilds: _zod.z.boolean().optional()
        }).optional(),
        excludeDefaultMomentLocales: _zod.z.boolean().optional(),
        experimental: _zod.z.strictObject({
            after: _zod.z.boolean().optional(),
            appDocumentPreloading: _zod.z.boolean().optional(),
            appIsrStatus: _zod.z.boolean().optional(),
            appNavFailHandling: _zod.z.boolean().optional(),
            preloadEntriesOnStart: _zod.z.boolean().optional(),
            allowedRevalidateHeaderKeys: _zod.z.array(_zod.z.string()).optional(),
            amp: _zod.z.object({
                // AMP optimizer option is unknown, use z.any() here
                optimizer: _zod.z.any().optional(),
                skipValidation: _zod.z.boolean().optional(),
                validator: _zod.z.string().optional()
            }).optional(),
            staleTimes: _zod.z.object({
                dynamic: _zod.z.number().optional(),
                static: _zod.z.number().optional()
            }).optional(),
            cacheLife: _zod.z.record(_zod.z.object({
                stale: _zod.z.number().optional(),
                revalidate: _zod.z.number().optional(),
                expire: _zod.z.number().optional()
            })).optional(),
            cacheHandlers: _zod.z.record(_zod.z.string(), _zod.z.string().optional()).optional(),
            clientRouterFilter: _zod.z.boolean().optional(),
            clientRouterFilterRedirects: _zod.z.boolean().optional(),
            clientRouterFilterAllowedRate: _zod.z.number().optional(),
            cpus: _zod.z.number().optional(),
            memoryBasedWorkersCount: _zod.z.boolean().optional(),
            craCompat: _zod.z.boolean().optional(),
            caseSensitiveRoutes: _zod.z.boolean().optional(),
            clientSegmentCache: _zod.z.boolean().optional(),
            disableOptimizedLoading: _zod.z.boolean().optional(),
            disablePostcssPresetEnv: _zod.z.boolean().optional(),
            dynamicIO: _zod.z.boolean().optional(),
            inlineCss: _zod.z.boolean().optional(),
            esmExternals: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.literal('loose')
            ]).optional(),
            serverActions: _zod.z.object({
                bodySizeLimit: zSizeLimit.optional(),
                allowedOrigins: _zod.z.array(_zod.z.string()).optional()
            }).optional(),
            // The original type was Record<string, any>
            extensionAlias: _zod.z.record(_zod.z.string(), _zod.z.any()).optional(),
            externalDir: _zod.z.boolean().optional(),
            externalMiddlewareRewritesResolve: _zod.z.boolean().optional(),
            fallbackNodePolyfills: _zod.z.literal(false).optional(),
            fetchCacheKeyPrefix: _zod.z.string().optional(),
            forceSwcTransforms: _zod.z.boolean().optional(),
            fullySpecified: _zod.z.boolean().optional(),
            gzipSize: _zod.z.boolean().optional(),
            imgOptConcurrency: _zod.z.number().int().optional().nullable(),
            imgOptTimeoutInSeconds: _zod.z.number().int().optional(),
            imgOptMaxInputPixels: _zod.z.number().int().optional(),
            imgOptSequentialRead: _zod.z.boolean().optional().nullable(),
            internal_disableSyncDynamicAPIWarnings: _zod.z.boolean().optional(),
            isrFlushToDisk: _zod.z.boolean().optional(),
            largePageDataBytes: _zod.z.number().optional(),
            linkNoTouchStart: _zod.z.boolean().optional(),
            manualClientBasePath: _zod.z.boolean().optional(),
            middlewarePrefetch: _zod.z.enum([
                'strict',
                'flexible'
            ]).optional(),
            multiZoneDraftMode: _zod.z.boolean().optional(),
            cssChunking: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.literal('strict')
            ]).optional(),
            nextScriptWorkers: _zod.z.boolean().optional(),
            // The critter option is unknown, use z.any() here
            optimizeCss: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.any()
            ]).optional(),
            optimisticClientCache: _zod.z.boolean().optional(),
            parallelServerCompiles: _zod.z.boolean().optional(),
            parallelServerBuildTraces: _zod.z.boolean().optional(),
            ppr: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.literal('incremental')
            ]).readonly().optional(),
            taint: _zod.z.boolean().optional(),
            reactOwnerStack: _zod.z.boolean().optional(),
            prerenderEarlyExit: _zod.z.boolean().optional(),
            proxyTimeout: _zod.z.number().gte(0).optional(),
            scrollRestoration: _zod.z.boolean().optional(),
            sri: _zod.z.object({
                algorithm: _zod.z.enum([
                    'sha256',
                    'sha384',
                    'sha512'
                ]).optional()
            }).optional(),
            strictNextHead: _zod.z.boolean().optional(),
            swcPlugins: _zod.z// The specific swc plugin's option is unknown, use z.any() here
            .array(_zod.z.tuple([
                _zod.z.string(),
                _zod.z.record(_zod.z.string(), _zod.z.any())
            ])).optional(),
            swcTraceProfiling: _zod.z.boolean().optional(),
            // NonNullable<webpack.Configuration['experiments']>['buildHttp']
            urlImports: _zod.z.any().optional(),
            workerThreads: _zod.z.boolean().optional(),
            webVitalsAttribution: _zod.z.array(_zod.z.union([
                _zod.z.literal('CLS'),
                _zod.z.literal('FCP'),
                _zod.z.literal('FID'),
                _zod.z.literal('INP'),
                _zod.z.literal('LCP'),
                _zod.z.literal('TTFB')
            ])).optional(),
            // This is partial set of mdx-rs transform options we support, aligned
            // with next_core::next_config::MdxRsOptions. Ensure both types are kept in sync.
            mdxRs: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.object({
                    development: _zod.z.boolean().optional(),
                    jsxRuntime: _zod.z.string().optional(),
                    jsxImportSource: _zod.z.string().optional(),
                    providerImportSource: _zod.z.string().optional(),
                    mdxType: _zod.z.enum([
                        'gfm',
                        'commonmark'
                    ]).optional()
                })
            ]).optional(),
            typedRoutes: _zod.z.boolean().optional(),
            webpackBuildWorker: _zod.z.boolean().optional(),
            webpackMemoryOptimizations: _zod.z.boolean().optional(),
            turbo: _zod.z.object({
                loaders: _zod.z.record(_zod.z.string(), _zod.z.array(zTurboLoaderItem)).optional(),
                rules: _zod.z.record(_zod.z.string(), zTurboRuleConfigItemOrShortcut).optional(),
                resolveAlias: _zod.z.record(_zod.z.string(), _zod.z.union([
                    _zod.z.string(),
                    _zod.z.array(_zod.z.string()),
                    _zod.z.record(_zod.z.string(), _zod.z.union([
                        _zod.z.string(),
                        _zod.z.array(_zod.z.string())
                    ]))
                ])).optional(),
                resolveExtensions: _zod.z.array(_zod.z.string()).optional(),
                treeShaking: _zod.z.boolean().optional(),
                persistentCaching: _zod.z.union([
                    _zod.z.number(),
                    _zod.z.literal(false)
                ]).optional(),
                memoryLimit: _zod.z.number().optional(),
                moduleIdStrategy: _zod.z.enum([
                    'named',
                    'deterministic'
                ]).optional(),
                minify: _zod.z.boolean().optional()
            }).optional(),
            optimizePackageImports: _zod.z.array(_zod.z.string()).optional(),
            optimizeServerReact: _zod.z.boolean().optional(),
            clientTraceMetadata: _zod.z.array(_zod.z.string()).optional(),
            serverMinification: _zod.z.boolean().optional(),
            serverSourceMaps: _zod.z.boolean().optional(),
            useWasmBinary: _zod.z.boolean().optional(),
            useLightningcss: _zod.z.boolean().optional(),
            useEarlyImport: _zod.z.boolean().optional(),
            testProxy: _zod.z.boolean().optional(),
            defaultTestRunner: _zod.z.enum(_nexttest.SUPPORTED_TEST_RUNNERS_LIST).optional(),
            allowDevelopmentBuild: _zod.z.literal(true).optional(),
            reactCompiler: _zod.z.union([
                _zod.z.boolean(),
                _zod.z.object({
                    compilationMode: _zod.z.enum([
                        'infer',
                        'annotation',
                        'all'
                    ]).optional(),
                    panicThreshold: _zod.z.enum([
                        'ALL_ERRORS',
                        'CRITICAL_ERRORS',
                        'NONE'
                    ]).optional()
                }).optional()
            ]),
            staticGenerationRetryCount: _zod.z.number().int().optional(),
            staticGenerationMaxConcurrency: _zod.z.number().int().optional(),
            staticGenerationMinPagesPerWorker: _zod.z.number().int().optional(),
            typedEnv: _zod.z.boolean().optional(),
            serverComponentsHmrCache: _zod.z.boolean().optional(),
            authInterrupts: _zod.z.boolean().optional()
        }).optional(),
        exportPathMap: _zod.z.function().args(zExportMap, _zod.z.object({
            dev: _zod.z.boolean(),
            dir: _zod.z.string(),
            outDir: _zod.z.string().nullable(),
            distDir: _zod.z.string(),
            buildId: _zod.z.string()
        })).returns(_zod.z.union([
            zExportMap,
            _zod.z.promise(zExportMap)
        ])).optional(),
        generateBuildId: _zod.z.function().args().returns(_zod.z.union([
            _zod.z.string(),
            _zod.z.null(),
            _zod.z.promise(_zod.z.union([
                _zod.z.string(),
                _zod.z.null()
            ]))
        ])).optional(),
        generateEtags: _zod.z.boolean().optional(),
        headers: _zod.z.function().args().returns(_zod.z.promise(_zod.z.array(zHeader))).optional(),
        httpAgentOptions: _zod.z.strictObject({
            keepAlive: _zod.z.boolean().optional()
        }).optional(),
        i18n: _zod.z.strictObject({
            defaultLocale: _zod.z.string().min(1),
            domains: _zod.z.array(_zod.z.strictObject({
                defaultLocale: _zod.z.string().min(1),
                domain: _zod.z.string().min(1),
                http: _zod.z.literal(true).optional(),
                locales: _zod.z.array(_zod.z.string().min(1)).optional()
            })).optional(),
            localeDetection: _zod.z.literal(false).optional(),
            locales: _zod.z.array(_zod.z.string().min(1))
        }).nullable().optional(),
        images: _zod.z.strictObject({
            localPatterns: _zod.z.array(_zod.z.strictObject({
                pathname: _zod.z.string().optional(),
                search: _zod.z.string().optional()
            })).max(25).optional(),
            remotePatterns: _zod.z.array(_zod.z.strictObject({
                hostname: _zod.z.string(),
                pathname: _zod.z.string().optional(),
                port: _zod.z.string().max(5).optional(),
                protocol: _zod.z.enum([
                    'http',
                    'https'
                ]).optional(),
                search: _zod.z.string().optional()
            })).max(50).optional(),
            unoptimized: _zod.z.boolean().optional(),
            contentSecurityPolicy: _zod.z.string().optional(),
            contentDispositionType: _zod.z.enum([
                'inline',
                'attachment'
            ]).optional(),
            dangerouslyAllowSVG: _zod.z.boolean().optional(),
            deviceSizes: _zod.z.array(_zod.z.number().int().gte(1).lte(10000)).max(25).optional(),
            disableStaticImages: _zod.z.boolean().optional(),
            domains: _zod.z.array(_zod.z.string()).max(50).optional(),
            formats: _zod.z.array(_zod.z.enum([
                'image/avif',
                'image/webp'
            ])).max(4).optional(),
            imageSizes: _zod.z.array(_zod.z.number().int().gte(1).lte(10000)).min(0).max(25).optional(),
            loader: _zod.z.enum(_imageconfig.VALID_LOADERS).optional(),
            loaderFile: _zod.z.string().optional(),
            minimumCacheTTL: _zod.z.number().int().gte(0).optional(),
            path: _zod.z.string().optional()
        }).optional(),
        logging: _zod.z.union([
            _zod.z.object({
                fetches: _zod.z.object({
                    fullUrl: _zod.z.boolean().optional(),
                    hmrRefreshes: _zod.z.boolean().optional()
                }).optional()
            }),
            _zod.z.literal(false)
        ]).optional(),
        modularizeImports: _zod.z.record(_zod.z.string(), _zod.z.object({
            transform: _zod.z.union([
                _zod.z.string(),
                _zod.z.record(_zod.z.string(), _zod.z.string())
            ]),
            preventFullImport: _zod.z.boolean().optional(),
            skipDefaultConversion: _zod.z.boolean().optional()
        })).optional(),
        onDemandEntries: _zod.z.strictObject({
            maxInactiveAge: _zod.z.number().optional(),
            pagesBufferLength: _zod.z.number().optional()
        }).optional(),
        output: _zod.z.enum([
            'standalone',
            'export'
        ]).optional(),
        outputFileTracingRoot: _zod.z.string().optional(),
        outputFileTracingExcludes: _zod.z.record(_zod.z.string(), _zod.z.array(_zod.z.string())).optional(),
        outputFileTracingIncludes: _zod.z.record(_zod.z.string(), _zod.z.array(_zod.z.string())).optional(),
        pageExtensions: _zod.z.array(_zod.z.string()).min(1).optional(),
        poweredByHeader: _zod.z.boolean().optional(),
        productionBrowserSourceMaps: _zod.z.boolean().optional(),
        publicRuntimeConfig: _zod.z.record(_zod.z.string(), _zod.z.any()).optional(),
        reactProductionProfiling: _zod.z.boolean().optional(),
        reactStrictMode: _zod.z.boolean().nullable().optional(),
        reactMaxHeadersLength: _zod.z.number().nonnegative().int().optional(),
        redirects: _zod.z.function().args().returns(_zod.z.promise(_zod.z.array(zRedirect))).optional(),
        rewrites: _zod.z.function().args().returns(_zod.z.promise(_zod.z.union([
            _zod.z.array(zRewrite),
            _zod.z.object({
                beforeFiles: _zod.z.array(zRewrite),
                afterFiles: _zod.z.array(zRewrite),
                fallback: _zod.z.array(zRewrite)
            })
        ]))).optional(),
        // sassOptions properties are unknown besides implementation, use z.any() here
        sassOptions: _zod.z.object({
            implementation: _zod.z.string().optional()
        }).catchall(_zod.z.any()).optional(),
        serverExternalPackages: _zod.z.array(_zod.z.string()).optional(),
        serverRuntimeConfig: _zod.z.record(_zod.z.string(), _zod.z.any()).optional(),
        skipMiddlewareUrlNormalize: _zod.z.boolean().optional(),
        skipTrailingSlashRedirect: _zod.z.boolean().optional(),
        staticPageGenerationTimeout: _zod.z.number().optional(),
        expireTime: _zod.z.number().optional(),
        target: _zod.z.string().optional(),
        trailingSlash: _zod.z.boolean().optional(),
        transpilePackages: _zod.z.array(_zod.z.string()).optional(),
        typescript: _zod.z.strictObject({
            ignoreBuildErrors: _zod.z.boolean().optional(),
            tsconfigPath: _zod.z.string().min(1).optional()
        }).optional(),
        useFileSystemPublicRoutes: _zod.z.boolean().optional(),
        // The webpack config type is unknown, use z.any() here
        webpack: _zod.z.any().nullable().optional(),
        watchOptions: _zod.z.strictObject({
            pollIntervalMs: _zod.z.number().positive().finite().optional()
        }).optional()
    }));

//# sourceMappingURL=config-schema.js.map