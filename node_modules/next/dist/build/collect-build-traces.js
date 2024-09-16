"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "collectBuildTraces", {
    enumerable: true,
    get: function() {
        return collectBuildTraces;
    }
});
const _trace = require("../trace");
const _nexttraceentrypointsplugin = require("./webpack/plugins/next-trace-entrypoints-plugin");
const _constants = require("../shared/lib/constants");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _utils = require("./utils");
const _swc = require("./swc");
const _nonnullable = require("../lib/non-nullable");
const _ciinfo = /*#__PURE__*/ _interop_require_wildcard(require("../telemetry/ci-info"));
const _debug = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/debug"));
const _picomatch = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/picomatch"));
const _requirehook = require("../server/require-hook");
const _nft = require("next/dist/compiled/@vercel/nft");
const _normalizepagepath = require("../shared/lib/page-path/normalize-page-path");
const _apppaths = require("../shared/lib/router/utils/app-paths");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../lib/is-error"));
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
const debug = (0, _debug.default)("next:build:build-traces");
function shouldIgnore(file, serverIgnoreFn, reasons, cachedIgnoreFiles, children = new Set()) {
    if (cachedIgnoreFiles.has(file)) {
        return cachedIgnoreFiles.get(file);
    }
    if (serverIgnoreFn(file)) {
        cachedIgnoreFiles.set(file, true);
        return true;
    }
    children.add(file);
    const reason = reasons.get(file);
    if (!reason || reason.parents.size === 0 || reason.type.includes("initial")) {
        cachedIgnoreFiles.set(file, false);
        return false;
    }
    // if all parents are ignored the child file
    // should be ignored as well
    let allParentsIgnored = true;
    for (const parent of reason.parents.values()){
        if (!children.has(parent)) {
            children.add(parent);
            if (!shouldIgnore(parent, serverIgnoreFn, reasons, cachedIgnoreFiles, children)) {
                allParentsIgnored = false;
                break;
            }
        }
    }
    cachedIgnoreFiles.set(file, allParentsIgnored);
    return allParentsIgnored;
}
async function collectBuildTraces({ dir, config, distDir, pageInfos, staticPages, nextBuildSpan = new _trace.Span({
    name: "build"
}), hasSsrAmpPages, buildTraceContext, outputFileTracingRoot }) {
    const startTime = Date.now();
    debug("starting build traces");
    let turboTasksForTrace;
    let bindings = await (0, _swc.loadBindings)();
    const runTurbotrace = async function() {
        if (!config.experimental.turbotrace || !buildTraceContext) {
            return;
        }
        if (!(bindings == null ? void 0 : bindings.isWasm) && typeof bindings.turbo.startTrace === "function") {
            var _config_experimental_turbotrace;
            let turbotraceOutputPath;
            let turbotraceFiles;
            turboTasksForTrace = bindings.turbo.createTurboTasks((((_config_experimental_turbotrace = config.experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace.memoryLimit) ?? _constants.TURBO_TRACE_DEFAULT_MEMORY_LIMIT) * 1024 * 1024);
            const { entriesTrace, chunksTrace } = buildTraceContext;
            if (entriesTrace) {
                const { appDir: buildTraceContextAppDir, depModArray, entryNameMap, outputPath, action } = entriesTrace;
                const depModSet = new Set(depModArray);
                const filesTracedInEntries = await bindings.turbo.startTrace(action, turboTasksForTrace);
                const { contextDirectory, input: entriesToTrace } = action;
                // only trace the assets under the appDir
                // exclude files from node_modules, entries and processed by webpack
                const filesTracedFromEntries = filesTracedInEntries.map((f)=>_path.default.join(contextDirectory, f)).filter((f)=>!f.includes("/node_modules/") && f.startsWith(buildTraceContextAppDir) && !entriesToTrace.includes(f) && !depModSet.has(f));
                if (filesTracedFromEntries.length) {
                    // The turbo trace doesn't provide the traced file type and reason at present
                    // let's write the traced files into the first [entry].nft.json
                    const [[, entryName]] = Array.from(Object.entries(entryNameMap)).filter(([k])=>k.startsWith(buildTraceContextAppDir));
                    const traceOutputPath = _path.default.join(outputPath, `../${entryName}.js.nft.json`);
                    const traceOutputDir = _path.default.dirname(traceOutputPath);
                    turbotraceOutputPath = traceOutputPath;
                    turbotraceFiles = filesTracedFromEntries.map((file)=>_path.default.relative(traceOutputDir, file));
                }
            }
            if (chunksTrace) {
                const { action, outputPath } = chunksTrace;
                action.input = action.input.filter((f)=>{
                    const outputPagesPath = _path.default.join(outputPath, "..", "pages");
                    return !f.startsWith(outputPagesPath) || !staticPages.includes(// strip `outputPagesPath` and file ext from absolute
                    f.substring(outputPagesPath.length, f.length - 3));
                });
                await bindings.turbo.startTrace(action, turboTasksForTrace);
                if (turbotraceOutputPath && turbotraceFiles) {
                    const existedNftFile = await _promises.default.readFile(turbotraceOutputPath, "utf8").then((existedContent)=>JSON.parse(existedContent)).catch(()=>({
                            version: _constants.TRACE_OUTPUT_VERSION,
                            files: []
                        }));
                    existedNftFile.files.push(...turbotraceFiles);
                    const filesSet = new Set(existedNftFile.files);
                    existedNftFile.files = [
                        ...filesSet
                    ];
                    await _promises.default.writeFile(turbotraceOutputPath, JSON.stringify(existedNftFile), "utf8");
                }
            }
        }
    };
    const { outputFileTracingIncludes = {}, outputFileTracingExcludes = {} } = config.experimental;
    const excludeGlobKeys = Object.keys(outputFileTracingExcludes);
    const includeGlobKeys = Object.keys(outputFileTracingIncludes);
    await nextBuildSpan.traceChild("node-file-trace-build", {
        isTurbotrace: Boolean(config.experimental.turbotrace) ? "true" : "false"
    }).traceAsyncFn(async ()=>{
        var _config_experimental_turbotrace, _config_experimental;
        const nextServerTraceOutput = _path.default.join(distDir, "next-server.js.nft.json");
        const nextMinimalTraceOutput = _path.default.join(distDir, "next-minimal-server.js.nft.json");
        const root = ((_config_experimental = config.experimental) == null ? void 0 : (_config_experimental_turbotrace = _config_experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace.contextDirectory) ?? outputFileTracingRoot;
        // Under standalone mode, we need to trace the extra IPC server and
        // worker files.
        const isStandalone = config.output === "standalone";
        const nextServerEntry = require.resolve("next/dist/server/next-server");
        const sharedEntriesSet = [
            ...config.experimental.turbotrace ? [] : Object.keys(_requirehook.defaultOverrides).map((value)=>require.resolve(value, {
                    paths: [
                        require.resolve("next/dist/server/require-hook")
                    ]
                }))
        ];
        const { cacheHandler } = config;
        // ensure we trace any dependencies needed for custom
        // incremental cache handler
        if (cacheHandler) {
            sharedEntriesSet.push(require.resolve(_path.default.isAbsolute(cacheHandler) ? cacheHandler : _path.default.join(dir, cacheHandler)));
        }
        const serverEntries = [
            ...sharedEntriesSet,
            ...isStandalone ? [
                require.resolve("next/dist/server/lib/start-server"),
                require.resolve("next/dist/server/next"),
                require.resolve("next/dist/server/require-hook")
            ] : [],
            require.resolve("next/dist/server/next-server")
        ].filter(Boolean);
        const minimalServerEntries = [
            ...sharedEntriesSet,
            require.resolve("next/dist/compiled/next-server/server.runtime.prod")
        ].filter(Boolean);
        const additionalIgnores = new Set();
        for (const glob of excludeGlobKeys){
            if ((0, _picomatch.default)(glob)("next-server")) {
                outputFileTracingExcludes[glob].forEach((exclude)=>{
                    additionalIgnores.add(exclude);
                });
            }
        }
        const makeIgnoreFn = (ignores)=>{
            // pre compile the ignore globs
            const isMatch = (0, _picomatch.default)(ignores, {
                contains: true,
                dot: true
            });
            return (pathname)=>{
                if (_path.default.isAbsolute(pathname) && !pathname.startsWith(root)) {
                    return true;
                }
                return isMatch(pathname);
            };
        };
        const sharedIgnores = [
            "**/next/dist/compiled/next-server/**/*.dev.js",
            ...isStandalone ? [] : [
                "**/next/dist/compiled/jest-worker/**/*"
            ],
            "**/next/dist/compiled/webpack/(bundle4|bundle5).js",
            "**/node_modules/webpack5/**/*",
            "**/next/dist/server/lib/route-resolver*",
            "next/dist/compiled/semver/semver/**/*.js",
            ..._ciinfo.hasNextSupport ? [
                // only ignore image-optimizer code when
                // this is being handled outside of next-server
                "**/next/dist/server/image-optimizer.js",
                "**/next/dist/server/lib/squoosh/**/*.wasm"
            ] : [],
            ...!hasSsrAmpPages ? [
                "**/next/dist/compiled/@ampproject/toolbox-optimizer/**/*"
            ] : [],
            ...isStandalone ? [] : _nexttraceentrypointsplugin.TRACE_IGNORES,
            ...additionalIgnores,
            ...config.experimental.outputFileTracingIgnores || []
        ];
        const sharedIgnoresFn = makeIgnoreFn(sharedIgnores);
        const serverIgnores = [
            ...sharedIgnores,
            "**/node_modules/react{,-dom,-dom-server-turbopack}/**/*.development.js",
            "**/*.d.ts",
            "**/*.map",
            "**/next/dist/pages/**/*",
            ..._ciinfo.hasNextSupport ? [
                "**/node_modules/sharp/**/*",
                "**/@img/sharp-libvips*/**/*"
            ] : []
        ].filter(_nonnullable.nonNullable);
        const serverIgnoreFn = makeIgnoreFn(serverIgnores);
        const minimalServerIgnores = [
            ...serverIgnores,
            "**/next/dist/compiled/edge-runtime/**/*",
            "**/next/dist/server/web/sandbox/**/*",
            "**/next/dist/server/post-process.js"
        ];
        const minimalServerIgnoreFn = makeIgnoreFn(minimalServerIgnores);
        const routesIgnores = [
            ...sharedIgnores,
            // server chunks are provided via next-trace-entrypoints-plugin plugin
            // as otherwise all chunks are traced here and included for all pages
            // whether they are needed or not
            "**/.next/server/chunks/**",
            "**/next/dist/server/optimize-amp.js",
            "**/next/dist/server/post-process.js"
        ].filter(_nonnullable.nonNullable);
        const routeIgnoreFn = makeIgnoreFn(routesIgnores);
        const traceContext = _path.default.join(nextServerEntry, "..", "..");
        const serverTracedFiles = new Set();
        const minimalServerTracedFiles = new Set();
        function addToTracedFiles(base, file, dest) {
            dest.add(_path.default.relative(distDir, _path.default.join(base, file)).replace(/\\/g, "/"));
        }
        if (isStandalone) {
            addToTracedFiles("", require.resolve("next/dist/compiled/jest-worker/processChild"), serverTracedFiles);
            addToTracedFiles("", require.resolve("next/dist/compiled/jest-worker/threadChild"), serverTracedFiles);
        }
        if (config.experimental.turbotrace) {
            await runTurbotrace();
            const startTrace = bindings.turbo.startTrace;
            const makeTrace = async (entries)=>{
                var _config_experimental_turbotrace, _config_experimental_turbotrace1, _config_experimental_turbotrace2, _config_experimental_turbotrace3;
                return startTrace({
                    action: "print",
                    input: entries,
                    contextDirectory: traceContext,
                    logLevel: (_config_experimental_turbotrace = config.experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace.logLevel,
                    processCwd: (_config_experimental_turbotrace1 = config.experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace1.processCwd,
                    logDetail: (_config_experimental_turbotrace2 = config.experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace2.logDetail,
                    showAll: (_config_experimental_turbotrace3 = config.experimental.turbotrace) == null ? void 0 : _config_experimental_turbotrace3.logAll
                }, turboTasksForTrace);
            };
            // turbotrace does not handle concurrent tracing
            const vanillaFiles = await makeTrace(serverEntries);
            const minimalFiles = await makeTrace(minimalServerEntries);
            for (const [set, files] of [
                [
                    serverTracedFiles,
                    vanillaFiles
                ],
                [
                    minimalServerTracedFiles,
                    minimalFiles
                ]
            ]){
                for (const file of files){
                    if (!(set === minimalServerTracedFiles ? minimalServerIgnoreFn : serverIgnoreFn)(_path.default.join(traceContext, file))) {
                        addToTracedFiles(traceContext, file, set);
                    }
                }
            }
        } else {
            var _buildTraceContext_chunksTrace;
            const chunksToTrace = [
                ...(buildTraceContext == null ? void 0 : (_buildTraceContext_chunksTrace = buildTraceContext.chunksTrace) == null ? void 0 : _buildTraceContext_chunksTrace.action.input) || [],
                ...serverEntries,
                ...minimalServerEntries
            ];
            const result = await (0, _nft.nodeFileTrace)(chunksToTrace, {
                base: outputFileTracingRoot,
                processCwd: dir,
                mixedModules: true,
                async readFile (p) {
                    try {
                        return await _promises.default.readFile(p, "utf8");
                    } catch (e) {
                        if ((0, _iserror.default)(e) && (e.code === "ENOENT" || e.code === "EISDIR")) {
                            // since tracing runs in parallel with static generation server
                            // files might be removed from that step so tolerate ENOENT
                            // errors gracefully
                            return "";
                        }
                        throw e;
                    }
                },
                async readlink (p) {
                    try {
                        return await _promises.default.readlink(p);
                    } catch (e) {
                        if ((0, _iserror.default)(e) && (e.code === "EINVAL" || e.code === "ENOENT" || e.code === "UNKNOWN")) {
                            return null;
                        }
                        throw e;
                    }
                },
                async stat (p) {
                    try {
                        return await _promises.default.stat(p);
                    } catch (e) {
                        if ((0, _iserror.default)(e) && (e.code === "ENOENT" || e.code === "ENOTDIR")) {
                            return null;
                        }
                        throw e;
                    }
                },
                // handle shared ignores at top-level as it
                // avoids over-tracing when we don't need to
                // and speeds up total trace time
                ignore (p) {
                    if (sharedIgnoresFn(p)) {
                        return true;
                    }
                    // if a chunk is attempting to be traced that isn't
                    // in our initial list we need to ignore it to prevent
                    // over tracing as webpack needs to be the source of
                    // truth for which chunks should be included for each entry
                    if (p.includes(".next/server/chunks") && !chunksToTrace.includes(_path.default.join(outputFileTracingRoot, p))) {
                        return true;
                    }
                    return false;
                }
            });
            const reasons = result.reasons;
            const fileList = result.fileList;
            for (const file of result.esmFileList){
                fileList.add(file);
            }
            const parentFilesMap = (0, _nexttraceentrypointsplugin.getFilesMapFromReasons)(fileList, reasons);
            const cachedLookupIgnore = new Map();
            const cachedLookupIgnoreMinimal = new Map();
            for (const [entries, tracedFiles] of [
                [
                    serverEntries,
                    serverTracedFiles
                ],
                [
                    minimalServerEntries,
                    minimalServerTracedFiles
                ]
            ]){
                for (const file of entries){
                    const curFiles = parentFilesMap.get(_path.default.relative(outputFileTracingRoot, file));
                    tracedFiles.add(_path.default.relative(distDir, file).replace(/\\/g, "/"));
                    for (const curFile of curFiles || []){
                        const filePath = _path.default.join(outputFileTracingRoot, curFile);
                        if (!shouldIgnore(curFile, tracedFiles === minimalServerTracedFiles ? minimalServerIgnoreFn : serverIgnoreFn, reasons, tracedFiles === minimalServerTracedFiles ? cachedLookupIgnoreMinimal : cachedLookupIgnore)) {
                            tracedFiles.add(_path.default.relative(distDir, filePath).replace(/\\/g, "/"));
                        }
                    }
                }
            }
            const { entryNameFilesMap } = (buildTraceContext == null ? void 0 : buildTraceContext.chunksTrace) || {};
            const cachedLookupIgnoreRoutes = new Map();
            await Promise.all([
                ...entryNameFilesMap ? Object.entries(entryNameFilesMap) : new Map()
            ].map(async ([entryName, entryNameFiles])=>{
                const isApp = entryName.startsWith("app/");
                const isPages = entryName.startsWith("pages/");
                let route = entryName;
                if (isApp) {
                    route = (0, _apppaths.normalizeAppPath)(route.substring("app".length));
                }
                if (isPages) {
                    route = (0, _normalizepagepath.normalizePagePath)(route.substring("pages".length));
                }
                // we don't need to trace for automatically statically optimized
                // pages as they don't have server bundles
                if (staticPages.includes(route)) {
                    return;
                }
                const entryOutputPath = _path.default.join(distDir, "server", `${entryName}.js`);
                const traceOutputPath = `${entryOutputPath}.nft.json`;
                const existingTrace = JSON.parse(await _promises.default.readFile(traceOutputPath, "utf8"));
                const traceOutputDir = _path.default.dirname(traceOutputPath);
                const curTracedFiles = new Set();
                for (const file of [
                    ...entryNameFiles,
                    entryOutputPath
                ]){
                    const curFiles = parentFilesMap.get(_path.default.relative(outputFileTracingRoot, file));
                    for (const curFile of curFiles || []){
                        if (!shouldIgnore(curFile, routeIgnoreFn, reasons, cachedLookupIgnoreRoutes)) {
                            const filePath = _path.default.join(outputFileTracingRoot, curFile);
                            const outputFile = _path.default.relative(traceOutputDir, filePath).replace(/\\/g, "/");
                            curTracedFiles.add(outputFile);
                        }
                    }
                }
                for (const file of existingTrace.files || []){
                    curTracedFiles.add(file);
                }
                await _promises.default.writeFile(traceOutputPath, JSON.stringify({
                    ...existingTrace,
                    files: [
                        ...curTracedFiles
                    ].sort()
                }));
            }));
        }
        const moduleTypes = [
            "app-page",
            "pages"
        ];
        for (const type of moduleTypes){
            const modulePath = require.resolve(`next/dist/server/future/route-modules/${type}/module.compiled`);
            const relativeModulePath = _path.default.relative(root, modulePath);
            const contextDir = _path.default.join(_path.default.dirname(modulePath), "vendored", "contexts");
            for (const item of (await _promises.default.readdir(contextDir))){
                const itemPath = _path.default.relative(root, _path.default.join(contextDir, item));
                if (!serverIgnoreFn(itemPath)) {
                    addToTracedFiles(root, itemPath, serverTracedFiles);
                    addToTracedFiles(root, itemPath, minimalServerTracedFiles);
                }
            }
            addToTracedFiles(root, relativeModulePath, serverTracedFiles);
            addToTracedFiles(root, relativeModulePath, minimalServerTracedFiles);
        }
        await Promise.all([
            _promises.default.writeFile(nextServerTraceOutput, JSON.stringify({
                version: 1,
                files: Array.from(serverTracedFiles)
            })),
            _promises.default.writeFile(nextMinimalTraceOutput, JSON.stringify({
                version: 1,
                files: Array.from(minimalServerTracedFiles)
            }))
        ]);
    });
    // apply outputFileTracingIncludes/outputFileTracingExcludes after runTurbotrace
    const includeExcludeSpan = nextBuildSpan.traceChild("apply-include-excludes");
    await includeExcludeSpan.traceAsyncFn(async ()=>{
        const globOrig = require("next/dist/compiled/glob");
        const glob = (pattern)=>{
            return new Promise((resolve, reject)=>{
                globOrig(pattern, {
                    cwd: dir,
                    nodir: true,
                    dot: true
                }, (err, files)=>{
                    if (err) {
                        return reject(err);
                    }
                    resolve(files);
                });
            });
        };
        const { entryNameFilesMap } = (buildTraceContext == null ? void 0 : buildTraceContext.chunksTrace) || {};
        const infos = pageInfos instanceof Map ? pageInfos : (0, _utils.deserializePageInfos)(pageInfos);
        await Promise.all([
            ...entryNameFilesMap ? Object.entries(entryNameFilesMap) : new Map()
        ].map(async ([entryName])=>{
            const isApp = entryName.startsWith("app/");
            const isPages = entryName.startsWith("pages/");
            let route = entryName;
            if (isApp) {
                route = (0, _apppaths.normalizeAppPath)(entryName);
            }
            if (isPages) {
                route = (0, _normalizepagepath.normalizePagePath)(entryName);
            }
            if (staticPages.includes(route)) {
                return;
            }
            // edge routes have no trace files
            const pageInfo = infos.get(route);
            if ((pageInfo == null ? void 0 : pageInfo.runtime) === "edge") {
                return;
            }
            const combinedIncludes = new Set();
            const combinedExcludes = new Set();
            for (const curGlob of includeGlobKeys){
                const isMatch = (0, _picomatch.default)(curGlob, {
                    dot: true,
                    contains: true
                });
                if (isMatch(route)) {
                    for (const include of outputFileTracingIncludes[curGlob]){
                        combinedIncludes.add(include.replace(/\\/g, "/"));
                    }
                }
            }
            for (const curGlob of excludeGlobKeys){
                const isMatch = (0, _picomatch.default)(curGlob, {
                    dot: true,
                    contains: true
                });
                if (isMatch(route)) {
                    for (const exclude of outputFileTracingExcludes[curGlob]){
                        combinedExcludes.add(exclude);
                    }
                }
            }
            if (!(combinedIncludes == null ? void 0 : combinedIncludes.size) && !(combinedExcludes == null ? void 0 : combinedExcludes.size)) {
                return;
            }
            const traceFile = _path.default.join(distDir, `server`, `${entryName}.js.nft.json`);
            const pageDir = _path.default.dirname(traceFile);
            const traceContent = JSON.parse(await _promises.default.readFile(traceFile, "utf8"));
            const includes = [];
            const resolvedTraceIncludes = new Map();
            if (combinedIncludes == null ? void 0 : combinedIncludes.size) {
                await Promise.all([
                    ...combinedIncludes
                ].map(async (includeGlob)=>{
                    const results = await glob(includeGlob);
                    const resolvedInclude = resolvedTraceIncludes.get(includeGlob) || [
                        ...results.map((file)=>{
                            return _path.default.relative(pageDir, _path.default.join(dir, file));
                        })
                    ];
                    includes.push(...resolvedInclude);
                    resolvedTraceIncludes.set(includeGlob, resolvedInclude);
                }));
            }
            const combined = new Set([
                ...traceContent.files,
                ...includes
            ]);
            if (combinedExcludes == null ? void 0 : combinedExcludes.size) {
                const resolvedGlobs = [
                    ...combinedExcludes
                ].map((exclude)=>_path.default.join(dir, exclude));
                // pre compile before forEach
                const isMatch = (0, _picomatch.default)(resolvedGlobs, {
                    dot: true,
                    contains: true
                });
                combined.forEach((file)=>{
                    if (isMatch(_path.default.join(pageDir, file))) {
                        combined.delete(file);
                    }
                });
            }
            // overwrite trace file with custom includes/excludes
            await _promises.default.writeFile(traceFile, JSON.stringify({
                version: traceContent.version,
                files: [
                    ...combined
                ]
            }));
        }));
    });
    debug(`finished build tracing ${Date.now() - startTime}ms`);
}

//# sourceMappingURL=collect-build-traces.js.map