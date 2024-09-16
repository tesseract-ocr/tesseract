import { red } from "../../lib/picocolors";
import formatWebpackMessages from "../../client/components/react-dev-overlay/internal/helpers/format-webpack-messages";
import { nonNullable } from "../../lib/non-nullable";
import { COMPILER_NAMES, CLIENT_STATIC_FILES_RUNTIME_MAIN_APP, APP_CLIENT_INTERNALS, PHASE_PRODUCTION_BUILD } from "../../shared/lib/constants";
import { runCompiler } from "../compiler";
import * as Log from "../output/log";
import getBaseWebpackConfig, { loadProjectInfo } from "../webpack-config";
import { TelemetryPlugin } from "../webpack/plugins/telemetry-plugin";
import { NextBuildContext, resumePluginState, getPluginState } from "../build-context";
import { createEntrypoints } from "../entries";
import loadConfig from "../../server/config";
import { getTraceEvents, initializeTraceState, setGlobal, trace } from "../../trace";
import { WEBPACK_LAYERS } from "../../lib/constants";
import { TraceEntryPointsPlugin } from "../webpack/plugins/next-trace-entrypoints-plugin";
import origDebug from "next/dist/compiled/debug";
import { Telemetry } from "../../telemetry/storage";
const debug = origDebug("next:build:webpack-build");
function isTelemetryPlugin(plugin) {
    return plugin instanceof TelemetryPlugin;
}
function isTraceEntryPointsPlugin(plugin) {
    return plugin instanceof TraceEntryPointsPlugin;
}
export async function webpackBuildImpl(compilerName) {
    var _clientConfig_plugins, _serverConfig_plugins;
    let result = {
        warnings: [],
        errors: [],
        stats: []
    };
    let webpackBuildStart;
    const nextBuildSpan = NextBuildContext.nextBuildSpan;
    const dir = NextBuildContext.dir;
    const config = NextBuildContext.config;
    const runWebpackSpan = nextBuildSpan.traceChild("run-webpack-compiler");
    const entrypoints = await nextBuildSpan.traceChild("create-entrypoints").traceAsyncFn(()=>createEntrypoints({
            buildId: NextBuildContext.buildId,
            config: config,
            envFiles: NextBuildContext.loadedEnvFiles,
            isDev: false,
            rootDir: dir,
            pageExtensions: config.pageExtensions,
            pagesDir: NextBuildContext.pagesDir,
            appDir: NextBuildContext.appDir,
            pages: NextBuildContext.mappedPages,
            appPaths: NextBuildContext.mappedAppPages,
            previewMode: NextBuildContext.previewProps,
            rootPaths: NextBuildContext.mappedRootPaths,
            hasInstrumentationHook: NextBuildContext.hasInstrumentationHook
        }));
    const commonWebpackOptions = {
        isServer: false,
        buildId: NextBuildContext.buildId,
        encryptionKey: NextBuildContext.encryptionKey,
        config: config,
        appDir: NextBuildContext.appDir,
        pagesDir: NextBuildContext.pagesDir,
        rewrites: NextBuildContext.rewrites,
        originalRewrites: NextBuildContext.originalRewrites,
        originalRedirects: NextBuildContext.originalRedirects,
        reactProductionProfiling: NextBuildContext.reactProductionProfiling,
        noMangling: NextBuildContext.noMangling,
        clientRouterFilters: NextBuildContext.clientRouterFilters,
        previewModeId: NextBuildContext.previewModeId,
        allowedRevalidateHeaderKeys: NextBuildContext.allowedRevalidateHeaderKeys,
        fetchCacheKeyPrefix: NextBuildContext.fetchCacheKeyPrefix
    };
    const configs = await runWebpackSpan.traceChild("generate-webpack-config").traceAsyncFn(async ()=>{
        const info = await loadProjectInfo({
            dir,
            config: commonWebpackOptions.config,
            dev: false
        });
        return Promise.all([
            getBaseWebpackConfig(dir, {
                ...commonWebpackOptions,
                middlewareMatchers: entrypoints.middlewareMatchers,
                runWebpackSpan,
                compilerType: COMPILER_NAMES.client,
                entrypoints: entrypoints.client,
                ...info
            }),
            getBaseWebpackConfig(dir, {
                ...commonWebpackOptions,
                runWebpackSpan,
                middlewareMatchers: entrypoints.middlewareMatchers,
                compilerType: COMPILER_NAMES.server,
                entrypoints: entrypoints.server,
                ...info
            }),
            getBaseWebpackConfig(dir, {
                ...commonWebpackOptions,
                runWebpackSpan,
                middlewareMatchers: entrypoints.middlewareMatchers,
                compilerType: COMPILER_NAMES.edgeServer,
                entrypoints: entrypoints.edgeServer,
                edgePreviewProps: {
                    __NEXT_PREVIEW_MODE_ID: NextBuildContext.previewProps.previewModeId,
                    __NEXT_PREVIEW_MODE_ENCRYPTION_KEY: NextBuildContext.previewProps.previewModeEncryptionKey,
                    __NEXT_PREVIEW_MODE_SIGNING_KEY: NextBuildContext.previewProps.previewModeSigningKey
                },
                ...info
            })
        ]);
    });
    const clientConfig = configs[0];
    const serverConfig = configs[1];
    const edgeConfig = configs[2];
    if (clientConfig.optimization && (clientConfig.optimization.minimize !== true || clientConfig.optimization.minimizer && clientConfig.optimization.minimizer.length === 0)) {
        Log.warn(`Production code optimization has been disabled in your project. Read more: https://nextjs.org/docs/messages/minification-disabled`);
    }
    webpackBuildStart = process.hrtime();
    debug(`starting compiler`, compilerName);
    // We run client and server compilation separately to optimize for memory usage
    await runWebpackSpan.traceAsyncFn(async ()=>{
        var _inputFileSystem_purge;
        // Run the server compilers first and then the client
        // compiler to track the boundary of server/client components.
        let clientResult = null;
        // During the server compilations, entries of client components will be
        // injected to this set and then will be consumed by the client compiler.
        let serverResult = null;
        let edgeServerResult = null;
        let inputFileSystem;
        if (!compilerName || compilerName === "server") {
            debug("starting server compiler");
            const start = Date.now();
            [serverResult, inputFileSystem] = await runCompiler(serverConfig, {
                runWebpackSpan,
                inputFileSystem
            });
            debug(`server compiler finished ${Date.now() - start}ms`);
        }
        if (!compilerName || compilerName === "edge-server") {
            debug("starting edge-server compiler");
            const start = Date.now();
            [edgeServerResult, inputFileSystem] = edgeConfig ? await runCompiler(edgeConfig, {
                runWebpackSpan,
                inputFileSystem
            }) : [
                null
            ];
            debug(`edge-server compiler finished ${Date.now() - start}ms`);
        }
        // Only continue if there were no errors
        if (!(serverResult == null ? void 0 : serverResult.errors.length) && !(edgeServerResult == null ? void 0 : edgeServerResult.errors.length)) {
            const pluginState = getPluginState();
            for(const key in pluginState.injectedClientEntries){
                const value = pluginState.injectedClientEntries[key];
                const clientEntry = clientConfig.entry;
                if (key === APP_CLIENT_INTERNALS) {
                    clientEntry[CLIENT_STATIC_FILES_RUNTIME_MAIN_APP] = {
                        import: [
                            // TODO-APP: cast clientEntry[CLIENT_STATIC_FILES_RUNTIME_MAIN_APP] to type EntryDescription once it's available from webpack
                            // @ts-expect-error clientEntry['main-app'] is type EntryDescription { import: ... }
                            ...clientEntry[CLIENT_STATIC_FILES_RUNTIME_MAIN_APP].import,
                            value
                        ],
                        layer: WEBPACK_LAYERS.appPagesBrowser
                    };
                } else {
                    clientEntry[key] = {
                        dependOn: [
                            CLIENT_STATIC_FILES_RUNTIME_MAIN_APP
                        ],
                        import: value,
                        layer: WEBPACK_LAYERS.appPagesBrowser
                    };
                }
            }
            if (!compilerName || compilerName === "client") {
                debug("starting client compiler");
                const start = Date.now();
                [clientResult, inputFileSystem] = await runCompiler(clientConfig, {
                    runWebpackSpan,
                    inputFileSystem
                });
                debug(`client compiler finished ${Date.now() - start}ms`);
            }
        }
        inputFileSystem == null ? void 0 : (_inputFileSystem_purge = inputFileSystem.purge) == null ? void 0 : _inputFileSystem_purge.call(inputFileSystem);
        result = {
            warnings: [
                ...(clientResult == null ? void 0 : clientResult.warnings) ?? [],
                ...(serverResult == null ? void 0 : serverResult.warnings) ?? [],
                ...(edgeServerResult == null ? void 0 : edgeServerResult.warnings) ?? []
            ].filter(nonNullable),
            errors: [
                ...(clientResult == null ? void 0 : clientResult.errors) ?? [],
                ...(serverResult == null ? void 0 : serverResult.errors) ?? [],
                ...(edgeServerResult == null ? void 0 : edgeServerResult.errors) ?? []
            ].filter(nonNullable),
            stats: [
                clientResult == null ? void 0 : clientResult.stats,
                serverResult == null ? void 0 : serverResult.stats,
                edgeServerResult == null ? void 0 : edgeServerResult.stats
            ]
        };
    });
    result = nextBuildSpan.traceChild("format-webpack-messages").traceFn(()=>formatWebpackMessages(result, true));
    const telemetryPlugin = (_clientConfig_plugins = clientConfig.plugins) == null ? void 0 : _clientConfig_plugins.find(isTelemetryPlugin);
    const traceEntryPointsPlugin = (_serverConfig_plugins = serverConfig.plugins) == null ? void 0 : _serverConfig_plugins.find(isTraceEntryPointsPlugin);
    const webpackBuildEnd = process.hrtime(webpackBuildStart);
    if (result.errors.length > 0) {
        // Only keep the first few errors. Others are often indicative
        // of the same problem, but confuse the reader with noise.
        if (result.errors.length > 5) {
            result.errors.length = 5;
        }
        let error = result.errors.filter(Boolean).join("\n\n");
        console.error(red("Failed to compile.\n"));
        if (error.indexOf("private-next-pages") > -1 && error.indexOf("does not contain a default export") > -1) {
            const page_name_regex = /'private-next-pages\/(?<page_name>[^']*)'/;
            const parsed = page_name_regex.exec(error);
            const page_name = parsed && parsed.groups && parsed.groups.page_name;
            throw new Error(`webpack build failed: found page without a React Component as default export in pages/${page_name}\n\nSee https://nextjs.org/docs/messages/page-without-valid-component for more info.`);
        }
        console.error(error);
        console.error();
        if (error.indexOf("private-next-pages") > -1 || error.indexOf("__next_polyfill__") > -1) {
            const err = new Error("webpack config.resolve.alias was incorrectly overridden. https://nextjs.org/docs/messages/invalid-resolve-alias");
            err.code = "INVALID_RESOLVE_ALIAS";
            throw err;
        }
        const err = new Error("Build failed because of webpack errors");
        err.code = "WEBPACK_ERRORS";
        throw err;
    } else {
        if (result.warnings.length > 0) {
            Log.warn("Compiled with warnings\n");
            console.warn(result.warnings.filter(Boolean).join("\n\n"));
            console.warn();
        } else if (!compilerName) {
            Log.event("Compiled successfully");
        }
        return {
            duration: webpackBuildEnd[0],
            buildTraceContext: traceEntryPointsPlugin == null ? void 0 : traceEntryPointsPlugin.buildTraceContext,
            pluginState: getPluginState(),
            telemetryState: {
                usages: (telemetryPlugin == null ? void 0 : telemetryPlugin.usages()) || [],
                packagesUsedInServerSideProps: (telemetryPlugin == null ? void 0 : telemetryPlugin.packagesUsedInServerSideProps()) || []
            }
        };
    }
}
// the main function when this file is run as a worker
export async function workerMain(workerData) {
    // Clone the telemetry for worker
    const telemetry = new Telemetry({
        distDir: workerData.buildContext.config.distDir
    });
    setGlobal("telemetry", telemetry);
    // setup new build context from the serialized data passed from the parent
    Object.assign(NextBuildContext, workerData.buildContext);
    // Initialize tracer state from the parent
    initializeTraceState(workerData.traceState);
    // Resume plugin state
    resumePluginState(NextBuildContext.pluginState);
    /// load the config because it's not serializable
    NextBuildContext.config = await loadConfig(PHASE_PRODUCTION_BUILD, NextBuildContext.dir);
    NextBuildContext.nextBuildSpan = trace(`worker-main-${workerData.compilerName}`);
    const result = await webpackBuildImpl(workerData.compilerName);
    const { entriesTrace, chunksTrace } = result.buildTraceContext ?? {};
    if (entriesTrace) {
        const { entryNameMap, depModArray } = entriesTrace;
        if (depModArray) {
            result.buildTraceContext.entriesTrace.depModArray = depModArray;
        }
        if (entryNameMap) {
            const entryEntries = entryNameMap;
            result.buildTraceContext.entriesTrace.entryNameMap = entryEntries;
        }
    }
    if (chunksTrace == null ? void 0 : chunksTrace.entryNameFilesMap) {
        const entryNameFilesMap = chunksTrace.entryNameFilesMap;
        result.buildTraceContext.chunksTrace.entryNameFilesMap = entryNameFilesMap;
    }
    NextBuildContext.nextBuildSpan.stop();
    return {
        ...result,
        debugTraceEvents: getTraceEvents()
    };
}

//# sourceMappingURL=impl.js.map