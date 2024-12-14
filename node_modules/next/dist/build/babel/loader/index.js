"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _transform = /*#__PURE__*/ _interop_require_default(require("./transform"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function nextBabelLoader(parentTrace, inputSource, inputSourceMap) {
    const filename = this.resourcePath;
    // Ensure `.d.ts` are not processed.
    if (filename.endsWith('.d.ts')) {
        return [
            inputSource,
            inputSourceMap
        ];
    }
    const target = this.target;
    const loaderOptions = parentTrace.traceChild('get-options')// @ts-ignore TODO: remove ignore once webpack 5 types are used
    .traceFn(()=>this.getOptions());
    if (loaderOptions.exclude && loaderOptions.exclude(filename)) {
        return [
            inputSource,
            inputSourceMap
        ];
    }
    const loaderSpanInner = parentTrace.traceChild('next-babel-turbo-transform');
    const { code: transformedSource, map: outputSourceMap } = loaderSpanInner.traceFn(()=>_transform.default.call(this, inputSource, inputSourceMap, loaderOptions, filename, target, loaderSpanInner));
    return [
        transformedSource,
        outputSourceMap
    ];
}
const nextBabelLoaderOuter = function nextBabelLoaderOuter(inputSource, inputSourceMap) {
    const callback = this.async();
    const loaderSpan = this.currentTraceSpan.traceChild('next-babel-turbo-loader');
    loaderSpan.traceAsyncFn(()=>nextBabelLoader.call(this, loaderSpan, inputSource, inputSourceMap)).then(([transformedSource, outputSourceMap])=>callback == null ? void 0 : callback(null, transformedSource, outputSourceMap || inputSourceMap), (err)=>{
        callback == null ? void 0 : callback(err);
    });
};
const _default = nextBabelLoaderOuter;

//# sourceMappingURL=index.js.map