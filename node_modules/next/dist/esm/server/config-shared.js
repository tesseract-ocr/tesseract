import os from 'os';
import { imageConfigDefault } from '../shared/lib/image-config';
import { INFINITE_CACHE } from '../lib/constants';
export const defaultConfig = {
    env: {},
    webpack: null,
    eslint: {
        ignoreDuringBuilds: false
    },
    typescript: {
        ignoreBuildErrors: false,
        tsconfigPath: 'tsconfig.json'
    },
    distDir: '.next',
    cleanDistDir: true,
    assetPrefix: '',
    cacheHandler: process.env.NEXT_CACHE_HANDLER_PATH,
    // default to 50MB limit
    cacheMaxMemorySize: 50 * 1024 * 1024,
    configOrigin: 'default',
    useFileSystemPublicRoutes: true,
    generateBuildId: ()=>null,
    generateEtags: true,
    pageExtensions: [
        'tsx',
        'ts',
        'jsx',
        'js'
    ],
    poweredByHeader: true,
    compress: true,
    images: imageConfigDefault,
    devIndicators: {
        appIsrStatus: true,
        buildActivity: true,
        buildActivityPosition: 'bottom-right'
    },
    onDemandEntries: {
        maxInactiveAge: 60 * 1000,
        pagesBufferLength: 5
    },
    amp: {
        canonicalBase: ''
    },
    basePath: '',
    sassOptions: {},
    trailingSlash: false,
    i18n: null,
    productionBrowserSourceMaps: false,
    excludeDefaultMomentLocales: true,
    serverRuntimeConfig: {},
    publicRuntimeConfig: {},
    reactProductionProfiling: false,
    reactStrictMode: null,
    reactMaxHeadersLength: 6000,
    httpAgentOptions: {
        keepAlive: true
    },
    logging: {},
    expireTime: process.env.__NEXT_TEST_MODE ? undefined : 31536000,
    staticPageGenerationTimeout: 60,
    output: !!process.env.NEXT_PRIVATE_STANDALONE ? 'standalone' : undefined,
    modularizeImports: undefined,
    outputFileTracingRoot: process.env.NEXT_PRIVATE_OUTPUT_TRACE_ROOT || '',
    experimental: {
        cacheLife: {
            default: {
                stale: undefined,
                revalidate: 60 * 15,
                expire: INFINITE_CACHE
            },
            seconds: {
                stale: undefined,
                revalidate: 1,
                expire: 60
            },
            minutes: {
                stale: 60 * 5,
                revalidate: 60,
                expire: 60 * 60
            },
            hours: {
                stale: 60 * 5,
                revalidate: 60 * 60,
                expire: 60 * 60 * 24
            },
            days: {
                stale: 60 * 5,
                revalidate: 60 * 60 * 24,
                expire: 60 * 60 * 24 * 7
            },
            weeks: {
                stale: 60 * 5,
                revalidate: 60 * 60 * 24 * 7,
                expire: 60 * 60 * 24 * 30
            },
            max: {
                stale: 60 * 5,
                revalidate: 60 * 60 * 24 * 30,
                expire: INFINITE_CACHE
            }
        },
        cacheHandlers: {
            default: process.env.NEXT_DEFAULT_CACHE_HANDLER_PATH,
            remote: process.env.NEXT_REMOTE_CACHE_HANDLER_PATH,
            static: process.env.NEXT_STATIC_CACHE_HANDLER_PATH
        },
        cssChunking: true,
        multiZoneDraftMode: false,
        appNavFailHandling: false,
        prerenderEarlyExit: true,
        serverMinification: true,
        serverSourceMaps: false,
        linkNoTouchStart: false,
        caseSensitiveRoutes: false,
        clientSegmentCache: false,
        appDocumentPreloading: undefined,
        preloadEntriesOnStart: true,
        clientRouterFilter: true,
        clientRouterFilterRedirects: false,
        fetchCacheKeyPrefix: '',
        middlewarePrefetch: 'flexible',
        optimisticClientCache: true,
        manualClientBasePath: false,
        cpus: Math.max(1, (Number(process.env.CIRCLE_NODE_TOTAL) || (os.cpus() || {
            length: 1
        }).length) - 1),
        memoryBasedWorkersCount: false,
        imgOptConcurrency: null,
        imgOptTimeoutInSeconds: 7,
        imgOptMaxInputPixels: 268402689,
        imgOptSequentialRead: null,
        isrFlushToDisk: true,
        workerThreads: false,
        proxyTimeout: undefined,
        optimizeCss: false,
        nextScriptWorkers: false,
        scrollRestoration: false,
        externalDir: false,
        disableOptimizedLoading: false,
        gzipSize: true,
        craCompat: false,
        esmExternals: true,
        fullySpecified: false,
        swcTraceProfiling: false,
        forceSwcTransforms: false,
        swcPlugins: undefined,
        largePageDataBytes: 128 * 1000,
        disablePostcssPresetEnv: undefined,
        amp: undefined,
        urlImports: undefined,
        turbo: undefined,
        typedRoutes: false,
        typedEnv: false,
        clientTraceMetadata: undefined,
        parallelServerCompiles: false,
        parallelServerBuildTraces: false,
        ppr: // TODO: remove once we've made PPR default
        // If we're testing, and the `__NEXT_EXPERIMENTAL_PPR` environment variable
        // has been set to `true`, enable the experimental PPR feature so long as it
        // wasn't explicitly disabled in the config.
        !!(process.env.__NEXT_TEST_MODE && process.env.__NEXT_EXPERIMENTAL_PPR === 'true'),
        authInterrupts: false,
        reactOwnerStack: false,
        webpackBuildWorker: undefined,
        webpackMemoryOptimizations: false,
        optimizeServerReact: true,
        useEarlyImport: false,
        staleTimes: {
            dynamic: 0,
            static: 300
        },
        allowDevelopmentBuild: undefined,
        reactCompiler: undefined,
        staticGenerationRetryCount: undefined,
        serverComponentsHmrCache: true,
        staticGenerationMaxConcurrency: 8,
        staticGenerationMinPagesPerWorker: 25,
        dynamicIO: false,
        inlineCss: false
    },
    bundlePagesRouterDependencies: false
};
export async function normalizeConfig(phase, config) {
    if (typeof config === 'function') {
        config = config(phase, {
            defaultConfig
        });
    }
    // Support `new Promise` and `async () =>` as return values of the config export
    return await config;
}

//# sourceMappingURL=config-shared.js.map