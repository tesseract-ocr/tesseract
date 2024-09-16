"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    webpackBuildImpl: null,
    workerMain: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    webpackBuildImpl: function() {
        return webpackBuildImpl;
    },
    workerMain: function() {
        return workerMain;
    }
});
const _picocolors = require("../../lib/picocolors");
const _formatwebpackmessages = /*#__PURE__*/ _interop_require_default(require("../../client/components/react-dev-overlay/internal/helpers/format-webpack-messages"));
const _nonnullable = require("../../lib/non-nullable");
const _constants = require("../../shared/lib/constants");
const _compiler = require("../compiler");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../output/log"));
const _webpackconfig = /*#__PURE__*/ _interop_require_wildcard(require("../webpack-config"));
const _telemetryplugin = require("../webpack/plugins/telemetry-plugin");
const _buildcontext = require("../build-context");
const _entries = require("../entries");
const _config = /*#__PURE__*/ _interop_require_default(require("../../server/config"));
const _trace = require("../../trace");
const _constants1 = require("../../lib/constants");
const _nexttraceentrypointsplugin = require("../webpack/plugins/next-trace-entrypoints-plugin");
const _debug = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/debug"));
const _storage = require("../../telemetry/storage");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const debug = (0, _debug.default)("next:build:webpack-build");
function isTelemetryPlugin(plugin) {
    return plugin instanceof _telemetryplugin.TelemetryPlugin;
}
function isTraceEntryPointsPlugin(plugin) {
    return plugin instanceof _nexttraceentrypointsplugin.TraceEntryPointsPlugin;
}
async function webpackBuildImpl(compilerName) {
    var _clientConfig_plugins, _serverConfig_plugins;
    let result = {
        warnings: [],
        errors: [],
        stats: []
    };
    let webpackBuildStart;
    const nextBuildSpan = _buildcontext.NextBuildContext.nextBuildSpan;
    const dir = _buildcontext.NextBuildContext.dir;
    const config = _buildcontext.NextBuildContext.config;
    const runWebpackSpan = nextBuildSpan.traceChild("run-webpack-compiler");
    const entrypoints = await nextBuildSpan.traceChild("create-entrypoints").traceAsyncFn(()=>(0, _entries.createEntrypoints)({
            buildId: _buildcontext.NextBuildContext.buildId,
            config: config,
            envFiles: _buildcontext.NextBuildContext.loadedEnvFiles,
            isDev: false,
            rootDir: dir,
            pageExtensions: config.pageExtensions,
            pagesDir: _buildcontext.NextBuildContext.pagesDir,
            appDir: _buildcontext.NextBuildContext.appDir,
            pages: _buildcontext.NextBuildContext.mappedPages,
            appPaths: _buildcontext.NextBuildContext.mappedAppPages,
            previewMode: _buildcontext.NextBuildContext.previewProps,
            rootPaths: _buildcontext.NextBuildContext.mappedRootPaths,
            hasInstrumentationHook: _buildcontext.NextBuildContext.hasInstrumentationHook
        }));
    const commonWebpackOptions = {
        isServer: false,
        buildId: _buildcontext.NextBuildContext.buildId,
        encryptionKey: _buildcontext.NextBuildContext.encryptionKey,
        config: config,
        appDir: _buildcontext.NextBuildContext.appDir,
        pagesDir: _buildcontext.NextBuildContext.pagesDir,
        rewrites: _buildcontext.NextBuildContext.rewrites,
        originalRewrites: _buildcontext.NextBuildContext.originalRewrites,
        originalRedirects: _buildcontext.NextBuildContext.originalRedirects,
        reactProductionProfiling: _buildcontext.NextBuildContext.reactProductionProfiling,
        noMangling: _buildcontext.NextBuildContext.noMangling,
        clientRouterFilters: _buildcontext.NextBuildContext.clientRouterFilters,
        previewModeId: _buildcontext.NextBuildContext.previewModeId,
        allowedRevalidateHeaderKeys: _buildcontext.NextBuildContext.allowedRevalidateHeaderKeys,
        fetchCacheKeyPrefix: _buildcontext.NextBuildContext.fetchCacheKeyPrefix
    };
    const configs = await runWebpackSpan.traceChild("generate-webpack-config").traceAsyncFn(async ()=>{
        const info = await (0, _webpackconfig.loadProjectInfo)({
            dir,
            config: commonWebpackOptions.config,
            dev: false
        });
        return Promise.all([
            (0, _webpackconfig.default)(dir, {
                ...commonWebpackOptions,
                middlewareMatchers: entrypoints.middlewareMatchers,
                runWebpackSpan,
                compilerType: _constants.COMPILER_NAMES.client,
                entrypoints: entrypoints.client,
                ...info
            }),
            (0, _webpackconfig.default)(dir, {
                ...commonWebpackOptions,
                runWebpackSpan,
                middlewareMatchers: entrypoints.middlewareMatchers,
                compilerType: _constants.COMPILER_NAMES.server,
                entrypoints: entrypoints.server,
                ...info
            }),
            (0, _webpackconfig.default)(dir, {
                ...commonWebpackOptions,
                runWebpackSpan,
                middlewareMatchers: entrypoints.middlewareMatchers,
                compilerType: _constants.COMPILER_NAMES.edgeServer,
                entrypoints: entrypoints.edgeServer,
                edgePreviewProps: {
                    __NEXT_PREVIEW_MODE_ID: _buildcontext.NextBuildContext.previewProps.previewModeId,
                    __NEXT_PREVIEW_MODE_ENCRYPTION_KEY: _buildcontext.NextBuildContext.previewProps.previewModeEncryptionKey,
                    __NEXT_PREVIEW_MODE_SIGNING_KEY: _buildcontext.NextBuildContext.previewProps.previewModeSigningKey
                },
                ...info
            })
        ]);
    });
    const clientConfig = configs[0];
    const serverConfig = configs[1];
    const edgeConfig = configs[2];
    if (clientConfig.optimization && (clientConfig.optimization.minimize !== true || clientConfig.optimization.minimizer && clientConfig.optimization.minimizer.length === 0)) {
        _log.warn(`Production code optimization has been disabled in your project. Read more: https://nextjs.org/docs/messages/minification-disabled`);
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
            [serverResult, inputFileSystem] = await (0, _compiler.runCompiler)(serverConfig, {
                runWebpackSpan,
                inputFileSystem
            });
            debug(`server compiler finished ${Date.now() - start}ms`);
        }
        if (!compilerName || compilerName === "edge-server") {
            debug("starting edge-server compiler");
            const start = Date.now();
            [edgeServerResult, inputFileSystem] = edgeConfig ? await (0, _compiler.runCompiler)(edgeConfig, {
                runWebpackSpan,
                inputFileSystem
            }) : [
                null
            ];
            debug(`edge-server compiler finished ${Date.now() - start}ms`);
        }
        // Only continue if there were no errors
        if (!(serverResult == null ? void 0 : serverResult.errors.length) && !(edgeServerResult == null ? void 0 : edgeServerResult.errors.length)) {
            const pluginState = (0, _buildcontext.getPluginState)();
            for(const key in pluginState.injectedClientEntries){
                const value = pluginState.injectedClientEntries[key];
                const clientEntry = clientConfig.entry;
                if (key === _constants.APP_CLIENT_INTERNALS) {
                    clientEntry[_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP] = {
                        import: [
                            // TODO-APP: cast clientEntry[CLIENT_STATIC_FILES_RUNTIME_MAIN_APP] to type EntryDescription once it's available from webpack
                            // @ts-expect-error clientEntry['main-app'] is type EntryDescription { import: ... }
                            ...clientEntry[_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP].import,
                            value
                        ],
                        layer: _constants1.WEBPACK_LAYERS.appPagesBrowser
                    };
                } else {
                    clientEntry[key] = {
                        dependOn: [
                            _constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP
                        ],
                        import: value,
                        layer: _constants1.WEBPACK_LAYERS.appPagesBrowser
                    };
                }
            }
            if (!compilerName || compilerName === "client") {
                debug("starting client compiler");
                const start = Date.now();
                [clientResult, inputFileSystem] = await (0, _compiler.runCompiler)(clientConfig, {
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
            ].filter(_nonnullable.nonNullable),
            errors: [
                ...(clientResult == null ? void 0 : clientResult.errors) ?? [],
                ...(serverResult == null ? void 0 : serverResult.errors) ?? [],
                ...(edgeServerResult == null ? void 0 : edgeServerResult.errors) ?? []
            ].filter(_nonnullable.nonNullable),
            stats: [
                clientResult == null ? void 0 : clientResult.stats,
                serverResult == null ? void 0 : serverResult.stats,
                edgeServerResult == null ? void 0 : edgeServerResult.stats
            ]
        };
    });
    result = nextBuildSpan.traceChild("format-webpack-messages").traceFn(()=>(0, _formatwebpackmessages.default)(result, true));
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
        console.error((0, _picocolors.red)("Failed to compile.\n"));
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
            _log.warn("Compiled with warnings\n");
            console.warn(result.warnings.filter(Boolean).join("\n\n"));
            console.warn();
        } else if (!compilerName) {
            _log.event("Compiled successfully");
        }
        return {
            duration: webpackBuildEnd[0],
            buildTraceContext: traceEntryPointsPlugin == null ? void 0 : traceEntryPointsPlugin.buildTraceContext,
            pluginState: (0, _buildcontext.getPluginState)(),
            telemetryState: {
                usages: (telemetryPlugin == null ? void 0 : telemetryPlugin.usages()) || [],
                packagesUsedInServerSideProps: (telemetryPlugin == null ? void 0 : telemetryPlugin.packagesUsedInServerSideProps()) || []
            }
        };
    }
}
async function workerMain(workerData) {
    // Clone the telemetry for worker
    const telemetry = new _storage.Telemetry({
        distDir: workerData.buildContext.config.distDir
    });
    (0, _trace.setGlobal)("telemetry", telemetry);
    // setup new build context from the serialized data passed from the parent
    Object.assign(_buildcontext.NextBuildContext, workerData.buildContext);
    // Initialize tracer state from the parent
    (0, _trace.initializeTraceState)(workerData.traceState);
    // Resume plugin state
    (0, _buildcontext.resumePluginState)(_buildcontext.NextBuildContext.pluginState);
    /// load the config because it's not serializable
    _buildcontext.NextBuildContext.config = await (0, _config.default)(_constants.PHASE_PRODUCTION_BUILD, _buildcontext.NextBuildContext.dir);
    _buildcontext.NextBuildContext.nextBuildSpan = (0, _trace.trace)(`worker-main-${workerData.compilerName}`);
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
    _buildcontext.NextBuildContext.nextBuildSpan.stop();
    return {
        ...result,
        debugTraceEvents: (0, _trace.getTraceEvents)()
    };
}

//# sourceMappingURL=impl.js.map