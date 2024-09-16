"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ProfilingPlugin: null,
    spans: null,
    webpackInvalidSpans: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ProfilingPlugin: function() {
        return ProfilingPlugin;
    },
    spans: function() {
        return spans;
    },
    webpackInvalidSpans: function() {
        return webpackInvalidSpans;
    }
});
const _webpack = require("next/dist/compiled/webpack/webpack");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const pluginName = "ProfilingPlugin";
const spans = new WeakMap();
const moduleSpansByCompilation = new WeakMap();
const makeSpanByCompilation = new WeakMap();
const sealSpanByCompilation = new WeakMap();
const webpackInvalidSpans = new WeakMap();
const TRACE_LABELS_SEAL = [
    "module assets",
    "create chunk assets",
    "asset render",
    "asset emit",
    "store asset"
];
function inTraceLabelsSeal(label) {
    return TRACE_LABELS_SEAL.some((l)=>label.startsWith(l));
}
class ProfilingPlugin {
    constructor({ runWebpackSpan, rootDir }){
        this.runWebpackSpan = runWebpackSpan;
        this.rootDir = rootDir;
    }
    apply(compiler) {
        this.traceTopLevelHooks(compiler);
        this.traceCompilationHooks(compiler);
        this.compiler = compiler;
    }
    traceHookPair(spanName, startHook, stopHook, { parentSpan, attrs, onStart, onStop } = {}) {
        let span;
        startHook.tap({
            name: pluginName,
            stage: -Infinity
        }, (...params)=>{
            const name = typeof spanName === "function" ? spanName() : spanName;
            const attributes = attrs ? attrs(...params) : attrs;
            span = parentSpan ? parentSpan(...params).traceChild(name, attributes) : this.runWebpackSpan.traceChild(name, attributes);
            if (onStart) onStart(span, ...params);
        });
        stopHook.tap({
            name: pluginName,
            stage: Infinity
        }, (...params)=>{
            // `stopHook` may be triggered when `startHook` has not in cases
            // where `stopHook` is used as the terminating event for more
            // than one pair of hooks.
            if (!span) {
                return;
            }
            if (onStop) onStop(span, ...params);
            span.stop();
        });
    }
    traceTopLevelHooks(compiler) {
        this.traceHookPair("webpack-compilation", compiler.hooks.compilation, compiler.hooks.afterCompile, {
            parentSpan: ()=>webpackInvalidSpans.get(compiler) || this.runWebpackSpan,
            attrs: ()=>({
                    name: compiler.name
                }),
            onStart: (span, compilation)=>{
                spans.set(compilation, span);
                spans.set(compiler, span);
                moduleSpansByCompilation.set(compilation, new WeakMap());
            }
        });
        if (compiler.options.mode === "development") {
            this.traceHookPair(()=>`webpack-invalidated-${compiler.name}`, compiler.hooks.invalid, compiler.hooks.done, {
                onStart: (span)=>webpackInvalidSpans.set(compiler, span),
                onStop: ()=>webpackInvalidSpans.delete(compiler),
                attrs: (fileName)=>({
                        trigger: fileName ? _path.default.relative(this.rootDir, fileName).replaceAll(_path.default.sep, "/") : "manual"
                    })
            });
        }
    }
    traceCompilationHooks(compiler) {
        this.traceHookPair("emit", compiler.hooks.emit, compiler.hooks.afterEmit, {
            parentSpan: ()=>webpackInvalidSpans.get(compiler) || this.runWebpackSpan
        });
        this.traceHookPair("make", compiler.hooks.make, compiler.hooks.finishMake, {
            parentSpan: (compilation)=>{
                const compilationSpan = spans.get(compilation);
                if (!compilationSpan) {
                    return webpackInvalidSpans.get(compiler) || this.runWebpackSpan;
                }
                return compilationSpan;
            },
            onStart: (span, compilation)=>{
                makeSpanByCompilation.set(compilation, span);
            },
            onStop: (_span, compilation)=>{
                makeSpanByCompilation.delete(compilation);
            }
        });
        compiler.hooks.compilation.tap({
            name: pluginName,
            stage: -Infinity
        }, (compilation)=>{
            compilation.hooks.buildModule.tap(pluginName, (module1)=>{
                var _compilation_moduleGraph;
                const moduleType = (()=>{
                    const r = module1.userRequest;
                    if (!r || r.endsWith("!")) {
                        return "";
                    } else {
                        const resource = r.split("!").pop();
                        const match = /^[^?]+\.([^?]+)$/.exec(resource);
                        return match ? match[1] : "";
                    }
                })();
                const issuerModule = compilation == null ? void 0 : (_compilation_moduleGraph = compilation.moduleGraph) == null ? void 0 : _compilation_moduleGraph.getIssuer(module1);
                let span;
                const moduleSpans = moduleSpansByCompilation.get(compilation);
                const spanName = `build-module${moduleType ? `-${moduleType}` : ""}`;
                const issuerSpan = issuerModule && (moduleSpans == null ? void 0 : moduleSpans.get(issuerModule));
                if (issuerSpan) {
                    span = issuerSpan.traceChild(spanName);
                } else {
                    let parentSpan;
                    for (const incomingConnection of compilation.moduleGraph.getIncomingConnections(module1)){
                        const entrySpan = spans.get(incomingConnection.dependency);
                        if (entrySpan) {
                            parentSpan = entrySpan;
                            break;
                        }
                    }
                    if (!parentSpan) {
                        const compilationSpan = spans.get(compilation);
                        if (!compilationSpan) {
                            return;
                        }
                        parentSpan = compilationSpan;
                    }
                    span = parentSpan.traceChild(spanName);
                }
                span.setAttribute("name", module1.userRequest);
                span.setAttribute("layer", module1.layer);
                moduleSpans.set(module1, span);
            });
            const moduleHooks = _webpack.NormalModule.getCompilationHooks(compilation);
            moduleHooks.readResource.for(undefined).intercept({
                register (tapInfo) {
                    const fn = tapInfo.fn;
                    tapInfo.fn = (loaderContext, callback)=>{
                        const moduleSpan = loaderContext.currentTraceSpan.traceChild(`read-resource`);
                        fn(loaderContext, (err, result)=>{
                            moduleSpan.stop();
                            callback(err, result);
                        });
                    };
                    return tapInfo;
                }
            });
            moduleHooks.loader.tap(pluginName, (loaderContext, module1)=>{
                var _moduleSpansByCompilation_get;
                const moduleSpan = (_moduleSpansByCompilation_get = moduleSpansByCompilation.get(compilation)) == null ? void 0 : _moduleSpansByCompilation_get.get(module1);
                loaderContext.currentTraceSpan = moduleSpan;
            });
            compilation.hooks.succeedModule.tap(pluginName, (module1)=>{
                var _moduleSpansByCompilation_get_get, _moduleSpansByCompilation_get;
                moduleSpansByCompilation == null ? void 0 : (_moduleSpansByCompilation_get = moduleSpansByCompilation.get(compilation)) == null ? void 0 : (_moduleSpansByCompilation_get_get = _moduleSpansByCompilation_get.get(module1)) == null ? void 0 : _moduleSpansByCompilation_get_get.stop();
            });
            compilation.hooks.failedModule.tap(pluginName, (module1)=>{
                var _moduleSpansByCompilation_get_get, _moduleSpansByCompilation_get;
                moduleSpansByCompilation == null ? void 0 : (_moduleSpansByCompilation_get = moduleSpansByCompilation.get(compilation)) == null ? void 0 : (_moduleSpansByCompilation_get_get = _moduleSpansByCompilation_get.get(module1)) == null ? void 0 : _moduleSpansByCompilation_get_get.stop();
            });
            this.traceHookPair("seal", compilation.hooks.seal, compilation.hooks.afterSeal, {
                parentSpan: ()=>spans.get(compilation),
                onStart (span) {
                    sealSpanByCompilation.set(compilation, span);
                },
                onStop () {
                    sealSpanByCompilation.delete(compilation);
                }
            });
            compilation.hooks.addEntry.tap(pluginName, (entry)=>{
                const parentSpan = makeSpanByCompilation.get(compilation) || spans.get(compilation);
                if (!parentSpan) {
                    return;
                }
                const addEntrySpan = parentSpan.traceChild("add-entry");
                addEntrySpan.setAttribute("request", entry.request);
                spans.set(entry, addEntrySpan);
            });
            compilation.hooks.succeedEntry.tap(pluginName, (entry)=>{
                var _spans_get;
                (_spans_get = spans.get(entry)) == null ? void 0 : _spans_get.stop();
                spans.delete(entry);
            });
            compilation.hooks.failedEntry.tap(pluginName, (entry)=>{
                var _spans_get;
                (_spans_get = spans.get(entry)) == null ? void 0 : _spans_get.stop();
                spans.delete(entry);
            });
            this.traceHookPair("chunk-graph", compilation.hooks.beforeChunks, compilation.hooks.afterChunks, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("optimize", compilation.hooks.optimize, compilation.hooks.reviveModules, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("optimize-modules", compilation.hooks.optimizeModules, compilation.hooks.afterOptimizeModules, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("optimize-chunks", compilation.hooks.optimizeChunks, compilation.hooks.afterOptimizeChunks, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("optimize-tree", compilation.hooks.optimizeTree, compilation.hooks.afterOptimizeTree, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("optimize-chunk-modules", compilation.hooks.optimizeChunkModules, compilation.hooks.afterOptimizeChunkModules, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("module-hash", compilation.hooks.beforeModuleHash, compilation.hooks.afterModuleHash, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("code-generation", compilation.hooks.beforeCodeGeneration, compilation.hooks.afterCodeGeneration, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("hash", compilation.hooks.beforeHash, compilation.hooks.afterHash, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            this.traceHookPair("code-generation-jobs", compilation.hooks.afterHash, compilation.hooks.beforeModuleAssets, {
                parentSpan: ()=>sealSpanByCompilation.get(compilation) || spans.get(compilation)
            });
            const logs = new Map();
            const originalTime = compilation.logger.time;
            const originalTimeEnd = compilation.logger.timeEnd;
            compilation.logger.time = (label)=>{
                if (!inTraceLabelsSeal(label)) {
                    return originalTime.call(compilation.logger, label);
                }
                const span = sealSpanByCompilation.get(compilation);
                if (span) {
                    logs.set(label, span.traceChild(label.replace(/ /g, "-")));
                }
                return originalTime.call(compilation.logger, label);
            };
            compilation.logger.timeEnd = (label)=>{
                if (!inTraceLabelsSeal(label)) {
                    return originalTimeEnd.call(compilation.logger, label);
                }
                const span = logs.get(label);
                if (span) {
                    span.stop();
                    logs.delete(label);
                }
                return originalTimeEnd.call(compilation.logger, label);
            };
        });
    }
}

//# sourceMappingURL=profiling-plugin.js.map