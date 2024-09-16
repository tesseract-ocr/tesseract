import os from "os";
import { imageConfigDefault } from "../shared/lib/image-config";
export const defaultConfig = {
    env: {},
    webpack: null,
    eslint: {
        ignoreDuringBuilds: false
    },
    typescript: {
        ignoreBuildErrors: false,
        tsconfigPath: "tsconfig.json"
    },
    distDir: ".next",
    cleanDistDir: true,
    assetPrefix: "",
    cacheHandler: undefined,
    // default to 50MB limit
    cacheMaxMemorySize: 50 * 1024 * 1024,
    configOrigin: "default",
    useFileSystemPublicRoutes: true,
    generateBuildId: ()=>null,
    generateEtags: true,
    pageExtensions: [
        "tsx",
        "ts",
        "jsx",
        "js"
    ],
    poweredByHeader: true,
    compress: true,
    analyticsId: process.env.VERCEL_ANALYTICS_ID || "",
    images: imageConfigDefault,
    devIndicators: {
        buildActivity: true,
        buildActivityPosition: "bottom-right"
    },
    onDemandEntries: {
        maxInactiveAge: 60 * 1000,
        pagesBufferLength: 5
    },
    amp: {
        canonicalBase: ""
    },
    basePath: "",
    sassOptions: {},
    trailingSlash: false,
    i18n: null,
    productionBrowserSourceMaps: false,
    optimizeFonts: true,
    excludeDefaultMomentLocales: true,
    serverRuntimeConfig: {},
    publicRuntimeConfig: {},
    reactProductionProfiling: false,
    reactStrictMode: null,
    httpAgentOptions: {
        keepAlive: true
    },
    outputFileTracing: true,
    staticPageGenerationTimeout: 60,
    swcMinify: true,
    output: !!process.env.NEXT_PRIVATE_STANDALONE ? "standalone" : undefined,
    modularizeImports: undefined,
    experimental: {
        multiZoneDraftMode: false,
        prerenderEarlyExit: false,
        serverMinification: true,
        serverSourceMaps: false,
        linkNoTouchStart: false,
        caseSensitiveRoutes: false,
        appDocumentPreloading: undefined,
        preloadEntriesOnStart: undefined,
        clientRouterFilter: true,
        clientRouterFilterRedirects: false,
        fetchCacheKeyPrefix: "",
        middlewarePrefetch: "flexible",
        optimisticClientCache: true,
        swrDelta: undefined,
        manualClientBasePath: false,
        cpus: Math.max(1, (Number(process.env.CIRCLE_NODE_TOTAL) || (os.cpus() || {
            length: 1
        }).length) - 1),
        memoryBasedWorkersCount: false,
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
        outputFileTracingRoot: process.env.NEXT_PRIVATE_OUTPUT_TRACE_ROOT || "",
        swcTraceProfiling: false,
        forceSwcTransforms: false,
        swcPlugins: undefined,
        largePageDataBytes: 128 * 1000,
        disablePostcssPresetEnv: undefined,
        amp: undefined,
        urlImports: undefined,
        adjustFontFallbacks: false,
        adjustFontFallbacksWithSizeAdjust: false,
        turbo: undefined,
        turbotrace: undefined,
        typedRoutes: false,
        instrumentationHook: false,
        bundlePagesExternals: false,
        parallelServerCompiles: false,
        parallelServerBuildTraces: false,
        ppr: // TODO: remove once we've made PPR default
        // If we're testing, and the `__NEXT_EXPERIMENTAL_PPR` environment variable
        // has been set to `true`, enable the experimental PPR feature so long as it
        // wasn't explicitly disabled in the config.
        process.env.__NEXT_TEST_MODE && process.env.__NEXT_EXPERIMENTAL_PPR === "true" ? true : false,
        webpackBuildWorker: undefined,
        missingSuspenseWithCSRBailout: true,
        optimizeServerReact: true,
        useEarlyImport: false,
        staleTimes: {
            dynamic: 30,
            static: 300
        }
    }
};
export async function normalizeConfig(phase, config) {
    if (typeof config === "function") {
        config = config(phase, {
            defaultConfig
        });
    }
    // Support `new Promise` and `async () =>` as return values of the config export
    return await config;
}

//# sourceMappingURL=config-shared.js.map