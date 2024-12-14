"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    TRACE_IGNORES: null,
    TraceEntryPointsPlugin: null,
    getFilesMapFromReasons: null,
    getHash: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    TRACE_IGNORES: function() {
        return TRACE_IGNORES;
    },
    TraceEntryPointsPlugin: function() {
        return TraceEntryPointsPlugin;
    },
    getFilesMapFromReasons: function() {
        return getFilesMapFromReasons;
    },
    getHash: function() {
        return getHash;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _crypto = /*#__PURE__*/ _interop_require_default(require("crypto"));
const _profilingplugin = require("./profiling-plugin");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../../lib/is-error"));
const _nft = require("next/dist/compiled/@vercel/nft");
const _constants = require("../../../shared/lib/constants");
const _webpack = require("next/dist/compiled/webpack/webpack");
const _webpackconfig = require("../../webpack-config");
const _picomatch = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/picomatch"));
const _getmodulebuildinfo = require("../loaders/get-module-build-info");
const _entries = require("../../entries");
const _handleexternals = require("../../handle-externals");
const _ismetadataroute = require("../../../lib/metadata/is-metadata-route");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const PLUGIN_NAME = 'TraceEntryPointsPlugin';
const TRACE_IGNORES = [
    '**/*/next/dist/server/next.js',
    '**/*/next/dist/bin/next'
];
const NOT_TRACEABLE = [
    '.wasm',
    '.png',
    '.jpg',
    '.jpeg',
    '.gif',
    '.webp',
    '.avif',
    '.ico',
    '.bmp',
    '.svg'
];
function getModuleFromDependency(compilation, dep) {
    return compilation.moduleGraph.getModule(dep);
}
function getFilesMapFromReasons(fileList, reasons, ignoreFn) {
    // this uses the reasons tree to collect files specific to a
    // certain parent allowing us to not have to trace each parent
    // separately
    const parentFilesMap = new Map();
    function propagateToParents(parents, file, seen = new Set()) {
        for (const parent of parents || []){
            if (!seen.has(parent)) {
                seen.add(parent);
                let parentFiles = parentFilesMap.get(parent);
                if (!parentFiles) {
                    parentFiles = new Map();
                    parentFilesMap.set(parent, parentFiles);
                }
                const ignored = Boolean(ignoreFn == null ? void 0 : ignoreFn(file, parent));
                parentFiles.set(file, {
                    ignored
                });
                const parentReason = reasons.get(parent);
                if (parentReason == null ? void 0 : parentReason.parents) {
                    propagateToParents(parentReason.parents, file, seen);
                }
            }
        }
    }
    for (const file of fileList){
        const reason = reasons.get(file);
        const isInitial = (reason == null ? void 0 : reason.type.length) === 1 && reason.type.includes('initial');
        if (!reason || !reason.parents || isInitial && reason.parents.size === 0) {
            continue;
        }
        propagateToParents(reason.parents, file);
    }
    return parentFilesMap;
}
function getHash(content) {
    return _crypto.default.createHash('sha1').update(content).digest('hex');
}
class TraceEntryPointsPlugin {
    constructor({ rootDir, appDir, pagesDir, compilerType, optOutBundlingPackages, appDirEnabled, traceIgnores, esmExternals, outputFileTracingRoot, swcLoaderConfig }){
        this.buildTraceContext = {};
        this.rootDir = rootDir;
        this.appDir = appDir;
        this.pagesDir = pagesDir;
        this.entryTraces = new Map();
        this.esmExternals = esmExternals;
        this.appDirEnabled = appDirEnabled;
        this.traceIgnores = traceIgnores || [];
        this.tracingRoot = outputFileTracingRoot || rootDir;
        this.optOutBundlingPackages = optOutBundlingPackages;
        this.traceHashes = new Map();
        this.compilerType = compilerType;
        this.swcLoaderConfig = swcLoaderConfig;
    }
    // Here we output all traced assets and webpack chunks to a
    // ${page}.js.nft.json file
    async createTraceAssets(compilation, assets, span) {
        const outputPath = compilation.outputOptions.path || '';
        await span.traceChild('create-trace-assets').traceAsyncFn(async ()=>{
            const entryFilesMap = new Map();
            const chunksToTrace = new Set();
            const entryNameFilesMap = new Map();
            const isTraceable = (file)=>!NOT_TRACEABLE.some((suffix)=>{
                    return file.endsWith(suffix);
                });
            for (const entrypoint of compilation.entrypoints.values()){
                const entryFiles = new Set();
                for (const chunk of entrypoint.getEntrypointChunk().getAllReferencedChunks()){
                    for (const file of chunk.files){
                        if (isTraceable(file)) {
                            const filePath = _path.default.join(outputPath, file);
                            chunksToTrace.add(filePath);
                            entryFiles.add(filePath);
                        }
                    }
                    for (const file of chunk.auxiliaryFiles){
                        if (isTraceable(file)) {
                            const filePath = _path.default.join(outputPath, file);
                            chunksToTrace.add(filePath);
                            entryFiles.add(filePath);
                        }
                    }
                }
                entryFilesMap.set(entrypoint, entryFiles);
                entryNameFilesMap.set(entrypoint.name || '', [
                    ...entryFiles
                ]);
            }
            // startTrace existed and callable
            this.buildTraceContext.chunksTrace = {
                action: {
                    action: 'annotate',
                    input: [
                        ...chunksToTrace
                    ],
                    contextDirectory: this.tracingRoot,
                    processCwd: this.rootDir
                },
                outputPath,
                entryNameFilesMap: Object.fromEntries(entryNameFilesMap)
            };
            // server compiler outputs to `server/chunks` so we traverse up
            // one, but edge-server does not so don't for that one
            const outputPrefix = this.compilerType === 'server' ? '../' : '';
            for (const [entrypoint, entryFiles] of entryFilesMap){
                var _this_entryTraces_get;
                const traceOutputName = `${outputPrefix}${entrypoint.name}.js.nft.json`;
                const traceOutputPath = _path.default.dirname(_path.default.join(outputPath, traceOutputName));
                // don't include the entry itself in the trace
                entryFiles.delete(_path.default.join(outputPath, `${outputPrefix}${entrypoint.name}.js`));
                if (entrypoint.name.startsWith('app/')) {
                    // Include the client reference manifest for pages and route handlers,
                    // excluding metadata route handlers.
                    const clientManifestsForEntrypoint = (0, _ismetadataroute.isMetadataRoute)(entrypoint.name) ? null : _path.default.join(outputPath, outputPrefix, entrypoint.name.replace(/%5F/g, '_') + '_' + _constants.CLIENT_REFERENCE_MANIFEST + '.js');
                    if (clientManifestsForEntrypoint !== null) {
                        entryFiles.add(clientManifestsForEntrypoint);
                    }
                }
                const finalFiles = [];
                await Promise.all([
                    ...new Set([
                        ...entryFiles,
                        ...((_this_entryTraces_get = this.entryTraces.get(entrypoint.name)) == null ? void 0 : _this_entryTraces_get.keys()) || []
                    ])
                ].map(async (file)=>{
                    var _this_entryTraces_get;
                    const fileInfo = (_this_entryTraces_get = this.entryTraces.get(entrypoint.name)) == null ? void 0 : _this_entryTraces_get.get(file);
                    const relativeFile = _path.default.relative(traceOutputPath, file).replace(/\\/g, '/');
                    if (file) {
                        if (!(fileInfo == null ? void 0 : fileInfo.bundled)) {
                            finalFiles.push(relativeFile);
                        }
                    }
                }));
                assets[traceOutputName] = new _webpack.sources.RawSource(JSON.stringify({
                    version: _constants.TRACE_OUTPUT_VERSION,
                    files: finalFiles
                }));
            }
        });
    }
    tapfinishModules(compilation, traceEntrypointsPluginSpan, doResolve, readlink, stat) {
        compilation.hooks.finishModules.tapAsync(PLUGIN_NAME, async (_stats, callback)=>{
            const finishModulesSpan = traceEntrypointsPluginSpan.traceChild('finish-modules');
            await finishModulesSpan.traceAsyncFn(async ()=>{
                // we create entry -> module maps so that we can
                // look them up faster instead of having to iterate
                // over the compilation modules list
                const entryNameMap = new Map();
                const entryModMap = new Map();
                const additionalEntries = new Map();
                const depModMap = new Map();
                await finishModulesSpan.traceChild('get-entries').traceAsyncFn(async ()=>{
                    for (const [name, entry] of compilation.entries.entries()){
                        const normalizedName = name == null ? void 0 : name.replace(/\\/g, '/');
                        const isPage = normalizedName.startsWith('pages/');
                        const isApp = this.appDirEnabled && normalizedName.startsWith('app/');
                        if (isApp || isPage) {
                            for (const dep of entry.dependencies){
                                if (!dep) continue;
                                const entryMod = getModuleFromDependency(compilation, dep);
                                // Handle case where entry is a loader coming from Next.js.
                                // For example edge-loader or app-loader.
                                if (entryMod && entryMod.resource === '') {
                                    const moduleBuildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(entryMod);
                                    // All loaders that are used to create entries have a `route` property on the buildInfo.
                                    if (moduleBuildInfo.route) {
                                        const absolutePath = (0, _entries.getPageFilePath)({
                                            absolutePagePath: moduleBuildInfo.route.absolutePagePath,
                                            rootDir: this.rootDir,
                                            appDir: this.appDir,
                                            pagesDir: this.pagesDir
                                        });
                                        // Ensures we don't handle non-pages.
                                        if (this.pagesDir && absolutePath.startsWith(this.pagesDir) || this.appDir && absolutePath.startsWith(this.appDir)) {
                                            entryModMap.set(absolutePath, entryMod);
                                            entryNameMap.set(absolutePath, name);
                                        }
                                    }
                                    // If there was no `route` property, we can assume that it was something custom instead.
                                    // In order to trace these we add them to the additionalEntries map.
                                    if (entryMod.request) {
                                        let curMap = additionalEntries.get(name);
                                        if (!curMap) {
                                            curMap = new Map();
                                            additionalEntries.set(name, curMap);
                                        }
                                        depModMap.set(entryMod.request, entryMod);
                                        curMap.set(entryMod.resource, entryMod);
                                    }
                                }
                                if (entryMod && entryMod.resource) {
                                    entryNameMap.set(entryMod.resource, name);
                                    entryModMap.set(entryMod.resource, entryMod);
                                    let curMap = additionalEntries.get(name);
                                    if (!curMap) {
                                        curMap = new Map();
                                        additionalEntries.set(name, curMap);
                                    }
                                    depModMap.set(entryMod.resource, entryMod);
                                    curMap.set(entryMod.resource, entryMod);
                                }
                            }
                        }
                    }
                });
                const readFile = async (path)=>{
                    var _mod_originalSource, _mod_originalSource1;
                    const mod = depModMap.get(path) || entryModMap.get(path);
                    // map the transpiled source when available to avoid
                    // parse errors in node-file-trace
                    let source = mod == null ? void 0 : (_mod_originalSource1 = mod.originalSource) == null ? void 0 : (_mod_originalSource = _mod_originalSource1.call(mod)) == null ? void 0 : _mod_originalSource.buffer();
                    return source || '';
                };
                const entryPaths = Array.from(entryModMap.keys());
                const collectDependencies = async (mod, parent)=>{
                    if (!mod || !mod.dependencies) return;
                    for (const dep of mod.dependencies){
                        const depMod = getModuleFromDependency(compilation, dep);
                        if ((depMod == null ? void 0 : depMod.resource) && !depModMap.get(depMod.resource)) {
                            depModMap.set(depMod.resource, depMod);
                            await collectDependencies(depMod, parent);
                        }
                    }
                };
                const entriesToTrace = [
                    ...entryPaths
                ];
                for (const entry of entryPaths){
                    await collectDependencies(entryModMap.get(entry), entry);
                    const entryName = entryNameMap.get(entry);
                    const curExtraEntries = additionalEntries.get(entryName);
                    if (curExtraEntries) {
                        entriesToTrace.push(...curExtraEntries.keys());
                    }
                }
                const contextDirectory = this.tracingRoot;
                const chunks = [
                    ...entriesToTrace
                ];
                this.buildTraceContext.entriesTrace = {
                    action: {
                        action: 'print',
                        input: chunks,
                        contextDirectory,
                        processCwd: this.rootDir
                    },
                    appDir: this.rootDir,
                    depModArray: Array.from(depModMap.keys()),
                    entryNameMap: Object.fromEntries(entryNameMap),
                    outputPath: compilation.outputOptions.path
                };
                let fileList;
                let reasons;
                const ignores = [
                    ...TRACE_IGNORES,
                    ...this.traceIgnores,
                    '**/node_modules/**'
                ];
                // pre-compile the ignore matcher to avoid repeating on every ignoreFn call
                const isIgnoreMatcher = (0, _picomatch.default)(ignores, {
                    contains: true,
                    dot: true
                });
                const ignoreFn = (path)=>{
                    return isIgnoreMatcher(path);
                };
                await finishModulesSpan.traceChild('node-file-trace-plugin', {
                    traceEntryCount: entriesToTrace.length + ''
                }).traceAsyncFn(async ()=>{
                    const result = await (0, _nft.nodeFileTrace)(entriesToTrace, {
                        base: this.tracingRoot,
                        processCwd: this.rootDir,
                        readFile,
                        readlink,
                        stat,
                        resolve: doResolve ? async (id, parent, job, isCjs)=>{
                            return doResolve(id, parent, job, !isCjs);
                        } : undefined,
                        ignore: ignoreFn,
                        mixedModules: true
                    });
                    // @ts-ignore
                    fileList = result.fileList;
                    result.esmFileList.forEach((file)=>fileList.add(file));
                    reasons = result.reasons;
                });
                await finishModulesSpan.traceChild('collect-traced-files').traceAsyncFn(()=>{
                    const parentFilesMap = getFilesMapFromReasons(fileList, reasons, (file)=>{
                        var _reasons_get;
                        // if a file was imported and a loader handled it
                        // we don't include it in the trace e.g.
                        // static image imports, CSS imports
                        file = _path.default.join(this.tracingRoot, file);
                        const depMod = depModMap.get(file);
                        const isAsset = (_reasons_get = reasons.get(_path.default.relative(this.tracingRoot, file))) == null ? void 0 : _reasons_get.type.includes('asset');
                        return !isAsset && Array.isArray(depMod == null ? void 0 : depMod.loaders) && depMod.loaders.length > 0;
                    });
                    for (const entry of entryPaths){
                        var _parentFilesMap_get;
                        const entryName = entryNameMap.get(entry);
                        const normalizedEntry = _path.default.relative(this.tracingRoot, entry);
                        const curExtraEntries = additionalEntries.get(entryName);
                        const finalDeps = new Map();
                        // ensure we include entry source file as well for
                        // hash comparison
                        finalDeps.set(entry, {
                            bundled: true
                        });
                        for (const [dep, info] of ((_parentFilesMap_get = parentFilesMap.get(normalizedEntry)) == null ? void 0 : _parentFilesMap_get.entries()) || []){
                            finalDeps.set(_path.default.join(this.tracingRoot, dep), {
                                bundled: info.ignored
                            });
                        }
                        if (curExtraEntries) {
                            for (const extraEntry of curExtraEntries.keys()){
                                var _parentFilesMap_get1;
                                const normalizedExtraEntry = _path.default.relative(this.tracingRoot, extraEntry);
                                finalDeps.set(extraEntry, {
                                    bundled: false
                                });
                                for (const [dep, info] of ((_parentFilesMap_get1 = parentFilesMap.get(normalizedExtraEntry)) == null ? void 0 : _parentFilesMap_get1.entries()) || []){
                                    finalDeps.set(_path.default.join(this.tracingRoot, dep), {
                                        bundled: info.ignored
                                    });
                                }
                            }
                        }
                        this.entryTraces.set(entryName, finalDeps);
                    }
                });
            }).then(()=>callback(), (err)=>callback(err));
        });
    }
    apply(compiler) {
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation)=>{
            const readlink = async (path)=>{
                try {
                    return await new Promise((resolve, reject)=>{
                        ;
                        compilation.inputFileSystem.readlink(path, (err, link)=>{
                            if (err) return reject(err);
                            resolve(link);
                        });
                    });
                } catch (e) {
                    if ((0, _iserror.default)(e) && (e.code === 'EINVAL' || e.code === 'ENOENT' || e.code === 'UNKNOWN')) {
                        return null;
                    }
                    throw e;
                }
            };
            const stat = async (path)=>{
                try {
                    return await new Promise((resolve, reject)=>{
                        ;
                        compilation.inputFileSystem.stat(path, (err, stats)=>{
                            if (err) return reject(err);
                            resolve(stats);
                        });
                    });
                } catch (e) {
                    if ((0, _iserror.default)(e) && (e.code === 'ENOENT' || e.code === 'ENOTDIR')) {
                        return null;
                    }
                    throw e;
                }
            };
            const compilationSpan = _profilingplugin.spans.get(compilation) || _profilingplugin.spans.get(compiler);
            const traceEntrypointsPluginSpan = compilationSpan.traceChild('next-trace-entrypoint-plugin');
            traceEntrypointsPluginSpan.traceFn(()=>{
                compilation.hooks.processAssets.tapAsync({
                    name: PLUGIN_NAME,
                    stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_SUMMARIZE
                }, (assets, callback)=>{
                    this.createTraceAssets(compilation, assets, traceEntrypointsPluginSpan).then(()=>callback()).catch((err)=>callback(err));
                });
                let resolver = compilation.resolverFactory.get('normal');
                function getPkgName(name) {
                    const segments = name.split('/');
                    if (name[0] === '@' && segments.length > 1) return segments.length > 1 ? segments.slice(0, 2).join('/') : null;
                    return segments.length ? segments[0] : null;
                }
                const getResolve = (options)=>{
                    const curResolver = resolver.withOptions(options);
                    return (parent, request, job)=>new Promise((resolve, reject)=>{
                            const context = _path.default.dirname(parent);
                            curResolver.resolve({}, context, request, {
                                fileDependencies: compilation.fileDependencies,
                                missingDependencies: compilation.missingDependencies,
                                contextDependencies: compilation.contextDependencies
                            }, async (err, result, resContext)=>{
                                if (err) return reject(err);
                                if (!result) {
                                    return reject(new Error('module not found'));
                                }
                                // webpack resolver doesn't strip loader query info
                                // from the result so use path instead
                                if (result.includes('?') || result.includes('!')) {
                                    result = (resContext == null ? void 0 : resContext.path) || result;
                                }
                                try {
                                    // we need to collect all parent package.json's used
                                    // as webpack's resolve doesn't expose this and parent
                                    // package.json could be needed for resolving e.g. stylis
                                    // stylis/package.json -> stylis/dist/umd/package.json
                                    if (result.includes('node_modules')) {
                                        let requestPath = result.replace(/\\/g, '/').replace(/\0/g, '');
                                        if (!_path.default.isAbsolute(request) && request.includes('/') && (resContext == null ? void 0 : resContext.descriptionFileRoot)) {
                                            var _getPkgName;
                                            requestPath = (resContext.descriptionFileRoot + request.slice(((_getPkgName = getPkgName(request)) == null ? void 0 : _getPkgName.length) || 0) + _path.default.sep + 'package.json').replace(/\\/g, '/').replace(/\0/g, '');
                                        }
                                        const rootSeparatorIndex = requestPath.indexOf('/');
                                        let separatorIndex;
                                        while((separatorIndex = requestPath.lastIndexOf('/')) > rootSeparatorIndex){
                                            requestPath = requestPath.slice(0, separatorIndex);
                                            const curPackageJsonPath = `${requestPath}/package.json`;
                                            if (await job.isFile(curPackageJsonPath)) {
                                                await job.emitFile(await job.realpath(curPackageJsonPath), 'resolve', parent);
                                            }
                                        }
                                    }
                                } catch (_err) {
                                // we failed to resolve the package.json boundary,
                                // we don't block emitting the initial asset from this
                                }
                                resolve([
                                    result,
                                    options.dependencyType === 'esm'
                                ]);
                            });
                        });
                };
                const CJS_RESOLVE_OPTIONS = {
                    ..._webpackconfig.NODE_RESOLVE_OPTIONS,
                    fullySpecified: undefined,
                    modules: undefined,
                    extensions: undefined
                };
                const BASE_CJS_RESOLVE_OPTIONS = {
                    ...CJS_RESOLVE_OPTIONS,
                    alias: false
                };
                const ESM_RESOLVE_OPTIONS = {
                    ..._webpackconfig.NODE_ESM_RESOLVE_OPTIONS,
                    fullySpecified: undefined,
                    modules: undefined,
                    extensions: undefined
                };
                const BASE_ESM_RESOLVE_OPTIONS = {
                    ...ESM_RESOLVE_OPTIONS,
                    alias: false
                };
                const doResolve = async (request, parent, job, isEsmRequested)=>{
                    const context = _path.default.dirname(parent);
                    // When in esm externals mode, and using import, we resolve with
                    // ESM resolving options.
                    const { res } = await (0, _handleexternals.resolveExternal)(this.rootDir, this.esmExternals, context, request, isEsmRequested, this.optOutBundlingPackages, (options)=>(_, resRequest)=>{
                            return getResolve(options)(parent, resRequest, job);
                        }, undefined, undefined, ESM_RESOLVE_OPTIONS, CJS_RESOLVE_OPTIONS, BASE_ESM_RESOLVE_OPTIONS, BASE_CJS_RESOLVE_OPTIONS);
                    if (!res) {
                        throw new Error(`failed to resolve ${request} from ${parent}`);
                    }
                    return res.replace(/\0/g, '');
                };
                this.tapfinishModules(compilation, traceEntrypointsPluginSpan, doResolve, readlink, stat);
            });
        });
    }
}

//# sourceMappingURL=next-trace-entrypoints-plugin.js.map