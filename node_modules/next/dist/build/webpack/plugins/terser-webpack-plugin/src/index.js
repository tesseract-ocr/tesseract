"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "TerserPlugin", {
    enumerable: true,
    get: function() {
        return TerserPlugin;
    }
});
const _path = /*#__PURE__*/ _interop_require_wildcard(require("path"));
const _webpack = require("next/dist/compiled/webpack/webpack");
const _plimit = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/p-limit"));
const _jestworker = require("next/dist/compiled/jest-worker");
const _profilingplugin = require("../../profiling-plugin");
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
function getEcmaVersion(environment) {
    // ES 6th
    if (environment.arrowFunction || environment.const || environment.destructuring || environment.forOf || environment.module) {
        return 2015;
    }
    // ES 11th
    if (environment.bigIntLiteral || environment.dynamicImport) {
        return 2020;
    }
    return 5;
}
function buildError(error, file) {
    if (error.line) {
        return new Error(`${file} from Terser\n${error.message} [${file}:${error.line},${error.col}]${error.stack ? `\n${error.stack.split("\n").slice(1).join("\n")}` : ""}`);
    }
    if (error.stack) {
        return new Error(`${file} from Terser\n${error.message}\n${error.stack}`);
    }
    return new Error(`${file} from Terser\n${error.message}`);
}
const debugMinify = process.env.NEXT_DEBUG_MINIFY;
class TerserPlugin {
    constructor(options = {}){
        const { terserOptions = {}, parallel, swcMinify } = options;
        this.options = {
            swcMinify,
            parallel,
            terserOptions
        };
    }
    async optimize(compiler, compilation, assets, optimizeOptions, cache, { SourceMapSource, RawSource }) {
        const compilationSpan = _profilingplugin.spans.get(compilation) || _profilingplugin.spans.get(compiler);
        const terserSpan = compilationSpan.traceChild("terser-webpack-plugin-optimize");
        terserSpan.setAttribute("compilationName", compilation.name);
        terserSpan.setAttribute("swcMinify", this.options.swcMinify);
        return terserSpan.traceAsyncFn(async ()=>{
            let numberOfAssetsForMinify = 0;
            const assetsList = Object.keys(assets);
            const assetsForMinify = await Promise.all(assetsList.filter((name)=>{
                if (!_webpack.ModuleFilenameHelpers.matchObject.bind(// eslint-disable-next-line no-undefined
                undefined, {
                    test: /\.[cm]?js(\?.*)?$/i
                })(name)) {
                    return false;
                }
                const res = compilation.getAsset(name);
                if (!res) {
                    console.log(name);
                    return false;
                }
                const { info } = res;
                // Skip double minimize assets from child compilation
                if (info.minimized) {
                    return false;
                }
                return true;
            }).map(async (name)=>{
                const { info, source } = compilation.getAsset(name);
                const eTag = cache.getLazyHashedEtag(source);
                const output = await cache.getPromise(name, eTag);
                if (!output) {
                    numberOfAssetsForMinify += 1;
                }
                if (debugMinify && debugMinify === "1") {
                    console.log(JSON.stringify({
                        name,
                        source: source.source().toString()
                    }), {
                        breakLength: Infinity,
                        maxStringLength: Infinity
                    });
                }
                return {
                    name,
                    info,
                    inputSource: source,
                    output,
                    eTag
                };
            }));
            const numberOfWorkers = Math.min(numberOfAssetsForMinify, optimizeOptions.availableNumberOfCores);
            let initializedWorker;
            // eslint-disable-next-line consistent-return
            const getWorker = ()=>{
                if (this.options.swcMinify) {
                    return {
                        minify: async (options)=>{
                            const result = await require("../../../../swc").minify(options.input, {
                                ...options.inputSourceMap ? {
                                    sourceMap: {
                                        content: JSON.stringify(options.inputSourceMap)
                                    }
                                } : {},
                                compress: true,
                                mangle: true,
                                output: {
                                    comments: false
                                }
                            });
                            return result;
                        }
                    };
                }
                if (initializedWorker) {
                    return initializedWorker;
                }
                initializedWorker = new _jestworker.Worker(_path.join(__dirname, "./minify.js"), {
                    numWorkers: numberOfWorkers,
                    enableWorkerThreads: true
                });
                initializedWorker.getStdout().pipe(process.stdout);
                initializedWorker.getStderr().pipe(process.stderr);
                return initializedWorker;
            };
            const limit = (0, _plimit.default)(// When using the SWC minifier the limit will be handled by Node.js
            this.options.swcMinify ? Infinity : numberOfAssetsForMinify > 0 ? numberOfWorkers : Infinity);
            const scheduledTasks = [];
            for (const asset of assetsForMinify){
                scheduledTasks.push(limit(async ()=>{
                    const { name, inputSource, info, eTag } = asset;
                    let { output } = asset;
                    const minifySpan = terserSpan.traceChild("minify-js");
                    minifySpan.setAttribute("name", name);
                    minifySpan.setAttribute("cache", typeof output === "undefined" ? "MISS" : "HIT");
                    return minifySpan.traceAsyncFn(async ()=>{
                        if (!output) {
                            const { source: sourceFromInputSource, map: inputSourceMap } = inputSource.sourceAndMap();
                            const input = Buffer.isBuffer(sourceFromInputSource) ? sourceFromInputSource.toString() : sourceFromInputSource;
                            const options = {
                                name,
                                input,
                                inputSourceMap,
                                terserOptions: {
                                    ...this.options.terserOptions
                                }
                            };
                            if (typeof options.terserOptions.module === "undefined") {
                                if (typeof info.javascriptModule !== "undefined") {
                                    options.terserOptions.module = info.javascriptModule;
                                } else if (/\.mjs(\?.*)?$/i.test(name)) {
                                    options.terserOptions.module = true;
                                } else if (/\.cjs(\?.*)?$/i.test(name)) {
                                    options.terserOptions.module = false;
                                }
                            }
                            try {
                                output = await getWorker().minify(options);
                            } catch (error) {
                                compilation.errors.push(buildError(error, name));
                                return;
                            }
                            if (output.map) {
                                output.source = new SourceMapSource(output.code, name, output.map, input, inputSourceMap, true);
                            } else {
                                output.source = new RawSource(output.code);
                            }
                            await cache.storePromise(name, eTag, {
                                source: output.source
                            });
                        }
                        const newInfo = {
                            minimized: true
                        };
                        const { source } = output;
                        compilation.updateAsset(name, source, newInfo);
                    });
                }));
            }
            await Promise.all(scheduledTasks);
            if (initializedWorker) {
                await initializedWorker.end();
            }
        });
    }
    apply(compiler) {
        var _compiler_webpack;
        const { SourceMapSource, RawSource } = (compiler == null ? void 0 : (_compiler_webpack = compiler.webpack) == null ? void 0 : _compiler_webpack.sources) || _webpack.sources;
        const { output } = compiler.options;
        if (typeof this.options.terserOptions.ecma === "undefined") {
            this.options.terserOptions.ecma = getEcmaVersion(output.environment || {});
        }
        const pluginName = this.constructor.name;
        const availableNumberOfCores = this.options.parallel;
        compiler.hooks.thisCompilation.tap(pluginName, (compilation)=>{
            const cache = compilation.getCache("TerserWebpackPlugin");
            const handleHashForChunk = (hash, _chunk)=>{
                // increment 'c' to invalidate cache
                hash.update("c");
            };
            const JSModulesHooks = _webpack.webpack.javascript.JavascriptModulesPlugin.getCompilationHooks(compilation);
            JSModulesHooks.chunkHash.tap(pluginName, (chunk, hash)=>{
                if (!chunk.hasRuntime()) return;
                return handleHashForChunk(hash, chunk);
            });
            compilation.hooks.processAssets.tapPromise({
                name: pluginName,
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_OPTIMIZE_SIZE
            }, (assets)=>this.optimize(compiler, compilation, assets, {
                    availableNumberOfCores
                }, cache, {
                    SourceMapSource,
                    RawSource
                }));
            compilation.hooks.statsPrinter.tap(pluginName, (stats)=>{
                stats.hooks.print.for("asset.info.minimized").tap("terser-webpack-plugin", (minimized, { green, formatFlag })=>// eslint-disable-next-line no-undefined
                    minimized ? green(formatFlag("minimized")) : undefined);
            });
        });
    }
}

//# sourceMappingURL=index.js.map