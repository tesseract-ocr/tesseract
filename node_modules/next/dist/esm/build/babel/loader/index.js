import transform from "./transform";
async function nextBabelLoader(parentTrace, inputSource, inputSourceMap) {
    const filename = this.resourcePath;
    const target = this.target;
    const loaderOptions = parentTrace.traceChild("get-options")// @ts-ignore TODO: remove ignore once webpack 5 types are used
    .traceFn(()=>this.getOptions());
    const loaderSpanInner = parentTrace.traceChild("next-babel-turbo-transform");
    const { code: transformedSource, map: outputSourceMap } = loaderSpanInner.traceFn(()=>transform.call(this, inputSource, inputSourceMap, loaderOptions, filename, target, loaderSpanInner));
    return [
        transformedSource,
        outputSourceMap
    ];
}
const nextBabelLoaderOuter = function nextBabelLoaderOuter(inputSource, inputSourceMap) {
    const callback = this.async();
    const loaderSpan = this.currentTraceSpan.traceChild("next-babel-turbo-loader");
    loaderSpan.traceAsyncFn(()=>nextBabelLoader.call(this, loaderSpan, inputSource, inputSourceMap)).then(([transformedSource, outputSourceMap])=>callback == null ? void 0 : callback(null, transformedSource, outputSourceMap || inputSourceMap), (err)=>{
        callback == null ? void 0 : callback(err);
    });
};
export default nextBabelLoaderOuter;

//# sourceMappingURL=index.js.map